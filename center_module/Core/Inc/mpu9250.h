#ifndef MPU9250_H
#define MPU9250_H

#include "stm32f4xx_hal.h"

// MPU9250寄存器地址定义
#define SMPLRT_DIV      0x19
#define CONFIG          0x1A
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define ACCEL_XOUT_H    0x3B
#define ACCEL_XOUT_L    0x3C
#define ACCEL_YOUT_H    0x3D
#define ACCEL_YOUT_L    0x3E
#define ACCEL_ZOUT_H    0x3F
#define ACCEL_ZOUT_L    0x40
#define GYRO_XOUT_H     0x43
#define GYRO_XOUT_L     0x44
#define GYRO_YOUT_H     0x45
#define GYRO_YOUT_L     0x46
#define GYRO_ZOUT_H     0x47
#define GYRO_ZOUT_L     0x48
#define PWR_MGMT_1      0x6B
#define WHO_AM_I        0x75

#ifdef __cplusplus
extern "C" {
#endif

// 函数声明
void MPU9250_GPIO_Init(void);
void MPU9250_I2C_Init(void);
void MPU9250_Init(void);
void MPU9250_ReadAccel(float* ax, float* ay, float* az);
void MPU9250_ReadGyro(float* gx, float* gy, float* gz);
void MPU9250_ReadMag(float* mx, float* my, float* mz);
void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float *roll, float *pitch, float *yaw);
float MPU9250_CalculateYaw(float q0, float q1, float q2, float q3);
float invSqrt(float number);
void calibrate(void);
uint8_t MPU_Get_Gyroscope(int16_t *gy, int16_t *gx, int16_t *gz);
uint8_t MPU_Get_Accelerometer(int16_t *ay, int16_t *ax, int16_t *az);
uint8_t MPU_Get_Magnetometer(int16_t *mx, int16_t *my, int16_t *mz);
uint8_t MPU_Get_Gyro(int16_t *igx, int16_t *igy, int16_t *igz, float *gx, float *gy, float *gz);
uint8_t MPU_Get_Accel(int16_t *iax, int16_t *iay, int16_t *iaz, float *ax, float *ay, float *az);
uint8_t MPU_Get_Mag(int16_t *imx, int16_t *imy, int16_t *imz, float *mx, float *my, float *mz);

// 外部变量声明
extern I2C_HandleTypeDef hi2c2;
extern float roll, pitch, yaw;
extern float q0, q1, q2, q3;
extern float exInt, eyInt, ezInt;
extern int16_t gyro_offsetx, gyro_offsety, gyro_offsetz;
extern int16_t accoldx, accoldy, accoldz;
extern int16_t magoldx, magoldy, magoldz;
extern int16_t gyrooldx, gyrooldy, gyrooldz;

#ifdef __cplusplus
}
#endif
#endif // MPU9250_H
