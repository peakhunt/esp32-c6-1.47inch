#pragma once

#include <stdint.h>
#include <stdbool.h>

#define IMU_CONFIG_MAGIC        0xdeadbeef
#define IMU_CONFIG_REVISION     1

typedef struct _imu_sensor_config_t
{
  int16_t   accel_off[3];
  int16_t   accel_scale[3];
  int16_t   gyro_off[3];
  int16_t   mag_bias[3];
  float     mag_dec;
} imu_sensor_config_t;

typedef struct _imu_config_t
{
  uint32_t              magic;
  uint16_t              revision;
  imu_sensor_config_t   sensor;
} imu_config_t;

extern void imu_config_init(void);
