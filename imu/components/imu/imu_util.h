#pragma once

#include "freertos/FreeRTOS.h"

static inline void
imu_msleep(int msec)
{
  vTaskDelay(pdMS_TO_TICKS(msec));
}

#define IMU_MSEC_TO_TICKS(msec)   pdMS_TO_TICKS(msec)
