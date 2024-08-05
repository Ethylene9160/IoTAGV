#include "mpu.h"
#include "i2c.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

QueueHandle_t MPU_Queue;

void I2C_Write(uint8_t devAddr, uint8_t regAddr, uint8_t data);
void I2C_Read(uint8_t devAddr, uint8_t regAddr, uint8_t* buffer, uint16_t length);
void MPU_Task(void *argument);

void MPU_Init(void) {
    uint8_t check;
    uint8_t data;
    char msg[128];

    // 创建队列
    MPU_Queue = xQueueCreate(10, sizeof(float));
    if (MPU_Queue == NULL) {
        snprintf(msg, sizeof(msg), "Queue creation failed\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        while(1); // 如果创建队列失败，则停止程序
    }

    // Check device ID WHO_AM_I
    I2C_Read(MPU6050_ADDR, WHO_AM_I_REG, &check, 1);
    if (check == 0x68) {
        snprintf(msg, sizeof(msg), "WHO_AM_I_REG = 0x%02X\r\n", check);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        // Power management register: wake up the MPU6050
        I2C_Write(MPU6050_ADDR, PWR_MGMT_1_REG, 0x00);

        // Set accelerometer configuration in ACCEL_CONFIG Register
        data = 0x00; // +/- 2g
        I2C_Write(MPU6050_ADDR, ACCEL_CONFIG_REG, data);

        // Set Gyroscopic configuration in GYRO_CONFIG Register
        data = 0x00; // +/- 250 degrees/s
        I2C_Write(MPU6050_ADDR, GYRO_CONFIG_REG, data);

        // Send initialization complete message to UART2
        snprintf(msg, sizeof(msg), "MPU6050 initialized and checked\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    MPU_Queue = xQueueCreate(10, sizeof(float));
}

void MPU_Read_Accel(float* ax, float* ay, float* az) {
    uint8_t buffer[6];
    int16_t raw_ax, raw_ay, raw_az;

    I2C_Read(MPU6050_ADDR, ACCEL_XOUT_H_REG, buffer, 6);

    raw_ax = (int16_t)(buffer[0] << 8) | buffer[1];
    raw_ay = (int16_t)(buffer[2] << 8) | buffer[3];
    raw_az = (int16_t)(buffer[4] << 8) | buffer[5];

    *ax = (float)raw_ax / 16384.0f;
    *ay = (float)raw_ay / 16384.0f;
    *az = (float)raw_az / 16384.0f;
}

void MPU_Read_Gyro(float* gx, float* gy, float* gz) {
    uint8_t buffer[6];
    int16_t raw_gx, raw_gy, raw_gz;

    I2C_Read(MPU6050_ADDR, GYRO_XOUT_H_REG, buffer, 6);

    raw_gx = (int16_t)(buffer[0] << 8) | buffer[1];
    raw_gy = (int16_t)(buffer[2] << 8) | buffer[3];
    raw_gz = (int16_t)(buffer[4] << 8) | buffer[5];

    *gx = (float)raw_gx / 131.0f;
    *gy = (float)raw_gy / 131.0f;
    *gz = (float)raw_gz / 131.0f;
}

float MPU_Calculate_Yaw(float gx, float dt) {
    static float yaw = 0.0f;
    yaw += gx * dt;
    return yaw;
}

void I2C_Write(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    uint8_t buffer[2];
    buffer[0] = regAddr;
    buffer[1] = data;
    HAL_I2C_Master_Transmit(&hi2c2, devAddr, buffer, 2, HAL_MAX_DELAY);
}

void I2C_Read(uint8_t devAddr, uint8_t regAddr, uint8_t* buffer, uint16_t length) {
    HAL_I2C_Master_Transmit(&hi2c2, devAddr, &regAddr, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c2, devAddr, buffer, length, HAL_MAX_DELAY);
}

void MPU_Process_Data(void) {
    static float alpha = 0.0f;
    float gx, gy, gz;

    MPU_Read_Gyro(&gx, &gy, &gz);
    alpha = MPU_Calculate_Yaw(gz, 0.01f); // assuming 0.01s time interval for simplicity

    char debug_msg[128];
    snprintf(debug_msg, sizeof(debug_msg), "Processing data: gx=%.2f, gy=%.2f, gz=%.2f, alpha=%.2f\r\n", gx, gy, gz, alpha);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    if (xQueueSendFromISR(MPU_Queue, &alpha, NULL) != pdTRUE) {
        Error_Handler();
    }
}

