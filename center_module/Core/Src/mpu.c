//
// Created by 10347 on 2024/8/4.
//
#include "MPU.h"
#include "i2c.h"

static void I2C_Write(uint8_t devAddr, uint8_t regAddr, uint8_t data);
static void I2C_Read(uint8_t devAddr, uint8_t regAddr, uint8_t* buffer, uint16_t length);

void MPU_Init(void) {
    uint8_t check;
    uint8_t data;

    // Check device ID WHO_AM_I
    I2C_Read(MPU6050_ADDR, WHO_AM_I_REG, &check, 1);
    if (check == 0x68) {
        // Power management register: wake up the MPU6050
        I2C_Write(MPU6050_ADDR, PWR_MGMT_1_REG, 0x00);

        // Set accelerometer configuration in ACCEL_CONFIG Register
        data = 0x00; // +/- 2g
        I2C_Write(MPU6050_ADDR, ACCEL_CONFIG_REG, data);

        // Set Gyroscopic configuration in GYRO_CONFIG Register
        data = 0x00; // +/- 250 degrees/s
        I2C_Write(MPU6050_ADDR, GYRO_CONFIG_REG, data);
    }
}

void MPU_Read_Accel(float* ax, float* ay, float* az) {
    uint8_t buffer[6];
    int16_t raw_ax, raw_ay, raw_az;

    I2C_Read(MPU6050_ADDR, ACCEL_XOUT_H_REG, buffer, 6);

    raw_ax = (buffer[0] << 8) | buffer[1];
    raw_ay = (buffer[2] << 8) | buffer[3];
    raw_az = (buffer[4] << 8) | buffer[5];

    *ax = (float)raw_ax / 16384.0f;
    *ay = (float)raw_ay / 16384.0f;
    *az = (float)raw_az / 16384.0f;
}

void MPU_Read_Gyro(float* gx, float* gy, float* gz) {
    uint8_t buffer[6];
    int16_t raw_gx, raw_gy, raw_gz;

    I2C_Read(MPU6050_ADDR, GYRO_XOUT_H_REG, buffer, 6);

    raw_gx = (buffer[0] << 8) | buffer[1];
    raw_gy = (buffer[2] << 8) | buffer[3];
    raw_gz = (buffer[4] << 8) | buffer[5];

    *gx = (float)raw_gx / 131.0f;
    *gy = (float)raw_gy / 131.0f;
    *gz = (float)raw_gz / 131.0f;
}

float MPU_Calculate_Yaw(float gx, float dt) {
    static float yaw = 0.0f;
    yaw += gx * dt;
    return yaw;
}

static void I2C_Write(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    uint8_t buffer[2];
    buffer[0] = regAddr;
    buffer[1] = data;
    HAL_I2C_Master_Transmit(&hi2c2, devAddr, buffer, 2, HAL_MAX_DELAY);
}

static void I2C_Read(uint8_t devAddr, uint8_t regAddr, uint8_t* buffer, uint16_t length) {
    HAL_I2C_Master_Transmit(&hi2c2, devAddr, &regAddr, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c2, devAddr, buffer, length, HAL_MAX_DELAY);
}
