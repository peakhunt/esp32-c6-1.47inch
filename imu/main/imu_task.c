#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mpu9250.h"
#include "imu.h"
#include "imu_task.h"
#include "web_server.h"

static const char *TAG = "imu_task";

static mpu9250_t              _mpu9250;
static imu_t                  _imu;
static SemaphoreHandle_t      _mutex;

volatile uint32_t             _sample_rate = 0;

static int64_t                      _cal_start_time = 0;
static const int64_t                CAL_DURATION_US = 60 * 1000000; // 60 seconds in microseconds
static imu_task_calib_complete_cb   _calib_cb = NULL;
static void*                        _calib_cb_arg = NULL;


static inline void
measure_sample_rate(void)
{
  static bool first = true;
  static uint32_t sample_count = 0;
  static uint64_t last_time = 0;
  uint64_t now;

  if(first)
  {
    first = false;
    last_time = esp_timer_get_time(); // microsecond
  }
  sample_count++;
  now = esp_timer_get_time();

  if (now - last_time >= 1000000ULL)
  {
    _sample_rate = (uint32_t)((float)sample_count / ((now - last_time) / 1000000.0f));
    // ESP_LOGI(TAG, "Sampling rate: %ld Hz", _sample_rate);
    sample_count = 0;
    last_time = now;
  }
}

static inline void
throttled_imu_data_send(void)
{
  static int skip_count = 0;

  // IMU task's target is 500Mz.
  // send data at 50Hz
  if (++skip_count >= 10)
  { 
    imu_telemetry_pkt_t pkt;
    extern volatile uint32_t cpu_usage;

    pkt.roll        = _imu.data.orientation[0];
    pkt.pitch       = _imu.data.orientation[1];
    pkt.yaw         = _imu.data.orientation[2];

    pkt.gx          = _imu.data.gyro[0];
    pkt.gy          = _imu.data.gyro[1];
    pkt.gz          = _imu.data.gyro[2];

    pkt.ax          = _imu.data.accel[0];
    pkt.ay          = _imu.data.accel[1];
    pkt.az          = _imu.data.accel[2];

    pkt.mx          = _imu.data.mag[0];
    pkt.my          = _imu.data.mag[1];
    pkt.mz          = _imu.data.mag[2];

    pkt.cpu_usage   = cpu_usage;
    pkt.sample_rate = _sample_rate;
    pkt.i2c_tx      = _mpu9250.num_transactions; 
    pkt.i2c_fail    = _mpu9250.num_failed;

    ws_broadcast_imu_update(&pkt);
    skip_count = 0;
  }
}

static bool
imu_task_handle_calibration_timer(void)
{
  int64_t current_time = esp_timer_get_time();
  int64_t elapsed = current_time - _cal_start_time;

  if(elapsed >= CAL_DURATION_US)
  {
    switch(_imu.mode)
    {
    case imu_mode_mag_calibrating:
      break;
    case imu_mode_gyro_calibrating:
      imu_gyro_calibration_finish(&_imu);
      ESP_LOGI(TAG, "finishing gyro calibration %ld %ld %ld",
          _imu.cal.gyro_off[0],
          _imu.cal.gyro_off[1],
          _imu.cal.gyro_off[2]);
      break;
    case imu_mode_accel_calibrating:
      break;
    default:
      break;
    }
    return true;
  }
  return false;
}

static void
imu_task(void *arg)
{
  ESP_LOGI(TAG, "Starting IMU task...");

  imu_init(&_imu, 500);
  mpu9250_init(&_mpu9250, MPU9250_Accelerometer_8G, MPU9250_Gyroscope_1000s, &_imu.lsb); 

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(2); // 2 ms → 500 Hz
  bool shouldNotify = false;

  imu_task_calib_complete_cb   calib_cb;
  void*                        calib_cb_arg;

  while (1)
  {
    mpu9250_read_all(&_mpu9250, &_imu.raw);

    xSemaphoreTake(_mutex, portMAX_DELAY);
    imu_update(&_imu);

    if(_imu.mode !=imu_mode_normal)
    {
      if(imu_task_handle_calibration_timer() == true)
      {
        calib_cb      = _calib_cb;
        calib_cb_arg  = _calib_cb_arg;
        shouldNotify  = true;
      }
    }
    xSemaphoreGive(_mutex);

    if(shouldNotify)
    {
      shouldNotify = false;
      calib_cb(calib_cb_arg);
    }

    measure_sample_rate();

    throttled_imu_data_send();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void
imu_task_init(void)
{
  _mutex = xSemaphoreCreateMutex();
  xTaskCreate(imu_task, "imu_task", 4096, NULL, 15, NULL);
}

void
imu_task_get_data(imu_data_t* data)
{
  xSemaphoreTake(_mutex, portMAX_DELAY);
  memcpy(data, &_imu.data, sizeof(imu_data_t));
  xSemaphoreGive(_mutex);
}

void
imu_task_get_attitude(float* roll, float* pitch, float* yaw)
{
  xSemaphoreTake(_mutex, portMAX_DELAY);
  *roll = _imu.data.orientation[0];
  *pitch = _imu.data.orientation[1];
  *yaw = _imu.data.orientation[2];
  xSemaphoreGive(_mutex);
}

void
imu_task_start_gyro_calibration(imu_task_calib_complete_cb cb, void* data)
{
  xSemaphoreTake(_mutex, portMAX_DELAY);
  _calib_cb = cb;
  _calib_cb_arg = data;
  imu_gyro_calibration_start(&_imu);
  _cal_start_time = esp_timer_get_time();
  ESP_LOGI(TAG, "starting gyro calibration");
  xSemaphoreGive(_mutex);
}

void
imu_task_get_gyro_calibration(float offset[3])
{
  xSemaphoreTake(_mutex, portMAX_DELAY);
  imu_gyro_get_calibration(&_imu, offset);
  xSemaphoreGive(_mutex);
}
