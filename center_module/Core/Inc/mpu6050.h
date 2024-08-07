// #ifndef MPU6050_H
// #define MPU6050_H
//
// #include "stm32f4xx_hal.h"
// #include "FreeRTOS.h"
// #include "queue.h"
//
// #define WHO_AM_I_REG        0x75
// #define PWR_MGMT_1_REG      0x6B
// #define ACCEL_CONFIG_REG    0x1C
// #define GYRO_CONFIG_REG     0x1B
// #define ACCEL_XOUT_H        0x3B
// #define ACCEL_XOUT_L        0x3C
// #define ACCEL_YOUT_H        0x3D
// #define ACCEL_YOUT_L        0x3E
// #define ACCEL_ZOUT_H        0x3F
// #define ACCEL_ZOUT_L        0x40
// #define GYRO_XOUT_H         0x43
// #define GYRO_XOUT_L         0x44
// #define GYRO_YOUT_H         0x45
// #define GYRO_YOUT_L         0x46
// #define GYRO_ZOUT_H         0x47
// #define GYRO_ZOUT_L         0x48
//
// void MPU6050_Init(void);
// void MPU6050_WriteReg(uint8_t regAddr, uint8_t data);
// uint8_t MPU6050_ReadReg(uint8_t regAddr);
// void MPU6050_ReadAccel(float* ax, float* ay, float* az);
// void MPU6050_ReadGyro(float* gx, float* gy, float* gz);
// float MPU6050_CalculateYaw(float gx, float dt);
// void MPU6050_ProcessData(void);
//
// extern QueueHandle_t MPU_Queue;
//
// #endif // MPU6050_H
