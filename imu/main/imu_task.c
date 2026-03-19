#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mpu9250.h"
#include "imu.h"
#include "web_server.h"

static const char *TAG = "imu_task";

static mpu9250_t              _mpu9250;
static imu_t                  _imu;
static SemaphoreHandle_t      _mutex;

volatile uint32_t             _sample_rate = 0;

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
    ws_broadcast_imu_update(_imu.data.orientation[0], 
                            _imu.data.orientation[1],
                            _imu.data.orientation[2]);
    skip_count = 0;
  }
}

static void
imu_task(void *arg)
{
  ESP_LOGI(TAG, "Starting IMU task...");

  imu_init(&_imu, 500);
  mpu9250_init(&_mpu9250, MPU9250_Accelerometer_8G, MPU9250_Gyroscope_1000s, &_imu.lsb); 

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(2); // 2 ms → 500 Hz

  while (1)
  {
    mpu9250_read_all(&_mpu9250, &_imu.raw);

    xSemaphoreTake(_mutex, portMAX_DELAY);
    imu_update(&_imu);
    xSemaphoreGive(_mutex);

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
