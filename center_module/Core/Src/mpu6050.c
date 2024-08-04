#include "mpu6050.h"
#include "i2c_software.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

#define MPU6050_ADDR (0x68 << 1)
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define ACCEL_CONFIG_REG 0x1C
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43

QueueHandle_t MPU_Queue;

void MPU6050_WriteReg(uint8_t regAddr, uint8_t data) {
    I2C_Software_Start();
    I2C_Software_WriteByte(MPU6050_ADDR);
    I2C_Software_ReadAck();
    I2C_Software_WriteByte(regAddr);
    I2C_Software_ReadAck();
    I2C_Software_WriteByte(data);
    I2C_Software_ReadAck();
    I2C_Software_Stop();
}

uint8_t MPU6050_ReadReg(uint8_t regAddr) {
    uint8_t data;

    I2C_Software_Start();
    I2C_Software_WriteByte(MPU6050_ADDR);
    I2C_Software_ReadAck();
    I2C_Software_WriteByte(regAddr);
    I2C_Software_ReadAck();

    I2C_Software_Restart();
    I2C_Software_WriteByte(MPU6050_ADDR | 0x01);
    I2C_Software_ReadAck();
    data = I2C_Software_ReadByte(0); // NACK after reading
    I2C_Software_Stop();

    return data;
}

void MPU6050_Init(void) {
    uint8_t check;
    char msg[128];

    I2C_Software_Init();

    snprintf(msg, sizeof(msg), "Starting MPU6050 Init\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // Check device ID WHO_AM_I
    check = MPU6050_ReadReg(WHO_AM_I_REG);
    snprintf(msg, sizeof(msg), "WHO_AM_I_REG = 0x%02X\r\n", check);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    if (check == 0x68) {
        // Power management register: wake up the MPU6050
        MPU6050_WriteReg(PWR_MGMT_1_REG, 0x00);
        HAL_UART_Transmit(&huart2, (uint8_t*)"PWR_MGMT_1_REG written\r\n", 24, HAL_MAX_DELAY);

        // Set accelerometer configuration in ACCEL_CONFIG Register
        MPU6050_WriteReg(ACCEL_CONFIG_REG, 0x00); // +/- 2g
        HAL_UART_Transmit(&huart2, (uint8_t*)"ACCEL_CONFIG_REG written\r\n", 27, HAL_MAX_DELAY);

        // Set Gyroscopic configuration in GYRO_CONFIG Register
        MPU6050_WriteReg(GYRO_CONFIG_REG, 0x00); // +/- 250 degrees/s
        HAL_UART_Transmit(&huart2, (uint8_t*)"GYRO_CONFIG_REG written\r\n", 26, HAL_MAX_DELAY);

        // Send initialization complete message to UART2
        snprintf(msg, sizeof(msg), "MPU6050 initialized and checked\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    } else {
        snprintf(msg, sizeof(msg), "MPU6050 not found\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    // 创建队列
    MPU_Queue = xQueueCreate(10, sizeof(float));
    if (MPU_Queue == NULL) {
        snprintf(msg, sizeof(msg), "Failed to create MPU_Queue\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
}

void MPU6050_ReadAccel(float* ax, float* ay, float* az) {
    uint8_t buffer[6];
    int16_t raw_ax, raw_ay, raw_az;

    I2C_Software_Start();
    I2C_Software_WriteByte(MPU6050_ADDR);
    I2C_Software_ReadAck();
    I2C_Software_WriteByte(ACCEL_XOUT_H);
    I2C_Software_ReadAck();

    I2C_Software_Restart();
    I2C_Software_WriteByte(MPU6050_ADDR | 0x01);
    I2C_Software_ReadAck();

    for (int i = 0; i < 6; i++) {
        buffer[i] = I2C_Software_ReadByte(i < 5 ? 1 : 0); // 最后一个字节发送 NACK
    }
    I2C_Software_Stop();

    raw_ax = (int16_t)(buffer[0] << 8 | buffer[1]);
    raw_ay = (int16_t)(buffer[2] << 8 | buffer[3]);
    raw_az = (int16_t)(buffer[4] << 8 | buffer[5]);

    *ax = (float)raw_ax / 16384.0f;
    *ay = (float)raw_ay / 16384.0f;
    *az = (float)raw_az / 16384.0f;
}

void MPU6050_ReadGyro(float* gx, float* gy, float* gz) {
    uint8_t buffer[6];
    int16_t raw_gx, raw_gy, raw_gz;

    I2C_Software_Start();
    I2C_Software_WriteByte(MPU6050_ADDR);
    I2C_Software_ReadAck();
    I2C_Software_WriteByte(GYRO_XOUT_H);
    I2C_Software_ReadAck();

    I2C_Software_Restart();
    I2C_Software_WriteByte(MPU6050_ADDR | 0x01);
    I2C_Software_ReadAck();

    for (int i = 0; i < 6; i++) {
        buffer[i] = I2C_Software_ReadByte(i < 5 ? 1 : 0); // 最后一个字节发送 NACK
    }
    I2C_Software_Stop();

    raw_gx = (int16_t)(buffer[0] << 8 | buffer[1]);
    raw_gy = (int16_t)(buffer[2] << 8 | buffer[3]);
    raw_gz = (int16_t)(buffer[4] << 8 | buffer[5]);

    *gx = (float)raw_gx / 131.0f;
    *gy = (float)raw_gy / 131.0f;
    *gz = (float)raw_gz / 131.0f;
}

float MPU6050_CalculateYaw(float gx, float dt) {
    static float yaw = 0.0f;
    yaw += gx * dt;
    return yaw;
}

void MPU6050_ProcessData(void) {
    float gx, gy, gz;
    float alpha;

    MPU6050_ReadGyro(&gx, &gy, &gz);
    alpha = MPU6050_CalculateYaw(gz, 0.01f); // assuming 0.01s time interval

    if (xQueueSendFromISR(MPU_Queue, &alpha, NULL) != pdTRUE) {
        Error_Handler();
    }
}
