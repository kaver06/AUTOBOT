#include "imu_data.h"

extern I2C_HandleTypeDef hi2c1;

// scale factors (from MPU6050 datasheet for ±4g, ±500dps)
#define ACC_SCALE   8192.0f   // LSB/g for ±4g
#define GYRO_SCALE  65.5f     // LSB/(°/s) for ±500dps
#define DT          0.01f     // sampling time in seconds (10ms for example)

IMU_Raw_t imu_raw;   // global struct to hold last raw values

// --- Read all sensor data ---
void imu_read_all(IMU_Raw_t *raw)
{
    uint8_t buffer[14];  // accel(6) + temp(2) + gyro(6)

    HAL_I2C_Mem_Read(&hi2c1, (DEVICE_ADDRESS<<1) | 0x01, REG_DATA, 1, buffer, 14, 100);

    raw->ax = (int16_t)(buffer[0] << 8 | buffer[1]);
    raw->ay = (int16_t)(buffer[2] << 8 | buffer[3]);
    raw->az = (int16_t)(buffer[4] << 8 | buffer[5]);
    raw->temp = (int16_t)(buffer[6] << 8 | buffer[7]);
    raw->gx = (int16_t)(buffer[8] << 8 | buffer[9]);
    raw->gy = (int16_t)(buffer[10] << 8 | buffer[11]);
    raw->gz = (int16_t)(buffer[12] << 8 | buffer[13]);
}

// --- Pose estimation (dx, dy, dtheta) ---
void get_pose_delta(float *dx, float *dy, float *dtheta, bool reset) {
    static float vx = 0.0f, vy = 0.0f, theta = 0.0f;

    if (reset) {
        vx = 0.0f;
        vy = 0.0f;
        theta = 0.0f;
    }

    imu_read_all(&imu_raw);

    float ax_g = (float)imu_raw.ax / ACC_SCALE;
    float ay_g = (float)imu_raw.ay / ACC_SCALE;
    float gz_dps = (float)imu_raw.gz / GYRO_SCALE;

    float ax_ms2 = ax_g * 9.81f;
    float ay_ms2 = ay_g * 9.81f;

    vx += ax_ms2 * DT;
    vy += ay_ms2 * DT;

    *dx = vx * DT;
    *dy = vy * DT;

    theta += gz_dps * DT;
    *dtheta = theta;
}

