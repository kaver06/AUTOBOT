/*
 * imu.c
 *
 *  Created on: Oct 22, 2025
 *      Author: kaver
 */

#include "imu.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;

MPU9250_Data imu;
uint8_t mpu_raw[14];
uint8_t mag_raw[7];
float mag_adjust[3];
static float yaw = 0.0f;
static uint32_t prev_time = 0;

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if(hi2c->Instance == I2C1) {
        mpu9250_parse_accel_gyro(mpu_raw);
        mpu9250_read_mag();
    }
}

void mpu9250_init(void) {
    uint8_t who_am_i = 0;
    HAL_I2C_Mem_Read(&hi2c1, MPU9250_ADDR, 0x75, 1, &who_am_i, 1, HAL_MAX_DELAY);

    if (who_am_i != 0x71 && who_am_i != 0x70)
    {
        printf("MPU not found! WHO_AM_I = 0x%X\r\n", who_am_i);
        return;
    } else
    {
        printf("MPU found! WHO_AM_I = 0x%X\r\n", who_am_i);
    }


    HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDR, PWR_MGMT_1_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x00}, 1, HAL_MAX_DELAY);
    HAL_Delay(100);

    HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDR, GYRO_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x00}, 1, HAL_MAX_DELAY);
    HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDR, ACCEL_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x00}, 1, HAL_MAX_DELAY);

    HAL_I2C_Mem_Write(&hi2c1, MPU9250_ADDR, INT_PIN_CFG, I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x02}, 1, HAL_MAX_DELAY);
    HAL_Delay(10);

    HAL_I2C_Mem_Write(&hi2c1, AK8963_ADDR, AK8963_CNTL1, I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x00}, 1, HAL_MAX_DELAY);
    HAL_Delay(10);
    HAL_I2C_Mem_Write(&hi2c1, AK8963_ADDR, AK8963_CNTL1, I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x0F}, 1, HAL_MAX_DELAY);
    HAL_Delay(10);

    uint8_t asa[3];
    HAL_I2C_Mem_Read(&hi2c1, AK8963_ADDR, AK8963_ASAX, I2C_MEMADD_SIZE_8BIT, asa, 3, HAL_MAX_DELAY);
    for(int i=0;i<3;i++) mag_adjust[i] = ((asa[i]-128)/256.0f)+1.0f;

    HAL_I2C_Mem_Write(&hi2c1, AK8963_ADDR, AK8963_CNTL1, I2C_MEMADD_SIZE_8BIT, (uint8_t[]){0x16}, 1, HAL_MAX_DELAY);
    HAL_Delay(10);

    printf("MPU9250 + AK8963 initialized!\r\n");
}

void mpu9250_parse_accel_gyro(uint8_t *raw) {
    int16_t ax = (raw[0]<<8)|raw[1];
    int16_t ay = (raw[2]<<8)|raw[3];
    int16_t az = (raw[4]<<8)|raw[5];
    int16_t gx = (raw[8]<<8)|raw[9];
    int16_t gy = (raw[10]<<8)|raw[11];
    int16_t gz = (raw[12]<<8)|raw[13];

    imu.ax = ax/16384.0f;
    imu.ay = ay/16384.0f;
    imu.az = az/16384.0f;
    imu.gx = gx/131.0f;
    imu.gy = gy/131.0f;
    imu.gz = gz/131.0f;

    imu.pitch = atan2f(imu.ay, sqrtf(imu.ax*imu.ax + imu.az*imu.az))*57.2958f;
    imu.roll  = atan2f(-imu.ax, imu.az)*57.2958f;
}

void mpu9250_read_mag(void) {
    // We don’t have a magnetometer, so compute yaw from gyro integration
    uint32_t now = HAL_GetTick();                // current time in ms
    float dt = (now - prev_time) / 1000.0f;      // seconds
    prev_time = now;

    // Integrate Z-axis gyro to get relative yaw (degrees)
    yaw += imu.gz * dt;

    // Normalize yaw to [0, 360)
    if (yaw >= 360.0f) yaw -= 360.0f;
    else if (yaw < 0.0f) yaw += 360.0f;

    imu.yaw = yaw;
}

