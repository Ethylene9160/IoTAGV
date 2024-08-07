#ifndef MPU_H
#define MPU_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "i2c.h"

#define MPU6050_ADDR            0x68<< 1
#define WHO_AM_I_REG            0x75
#define PWR_MGMT_1_REG          0x6B
#define ACCEL_XOUT_H_REG        0x3B
#define GYRO_XOUT_H_REG         0x43
#define ACCEL_CONFIG_REG        0x1C
#define GYRO_CONFIG_REG         0x1B
extern QueueHandle_t MPU_Queue;

#ifdef __cplusplus
extern "C" {
#endif

  void MPU_Init(void);
  void MPU_Read_Accel(float* ax, float* ay, float* az);
  void MPU_Read_Gyro(float* gx, float* gy, float* gz);
  // void MPU_Process_Data(void);
  // void MPU_Task(void *argument);

#ifdef __cplusplus
  }
#endif
#endif
