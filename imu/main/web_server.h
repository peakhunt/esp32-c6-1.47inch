#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
    float roll;             // 0-3
    float pitch;            // 4-7
    float yaw;              // 8-11
    int32_t cpu_usage;      // 12-15
    int32_t sample_rate;    // 16-19
    uint64_t i2c_tx;        // 20-27
    uint64_t i2c_fail;      // 28-35
} imu_telemetry_pkt_t;

extern void web_server_init(void);
extern void ws_broadcast_imu_update(imu_telemetry_pkt_t* pkt);
