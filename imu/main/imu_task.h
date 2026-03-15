#pragma once

#include "imu.h"

extern void imu_task_init(void);
extern void imu_task_get_data(imu_data_t* data);
extern void imu_task_get_attitude(float* roll, float* pitch, float* yaw);

extern volatile uint32_t _sample_rate;
