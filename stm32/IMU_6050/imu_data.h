#ifndef INC_IMU_DATA_H_
#define INC_IMU_DATA_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <stdbool.h>

// IMU I2C device
#define DEVICE_ADDRESS 0x68

// Registers
#define REG_CONFIG_GYRO  27
#define REG_CONFIG_ACC   28
#define REG_USR_CTRL     107
#define REG_DATA         59  // starting register for accel+gyro block

// Sensitivity settings
#define FS_GYRO_250     0
#define FS_GYRO_500     8
#define FS_GYRO_1000    9
#define FS_GYRO_2000    10

#define FS_ACC_2G       0
#define FS_ACC_4G       8
#define FS_ACC_8G       9
#define FS_ACC_16G      10

// Raw IMU data structure
typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t temp;
} IMU_Raw_t;

void init_imu(void);
void imu_read_all(IMU_Raw_t *raw);
void get_pose_delta(float *dx, float *dy, float *dtheta, bool reset);

#endif /* INC_IMU_DATA_H_ */
