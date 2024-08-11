#include "stm32f4xx_hal.h"
#include "mpu9250.h"
#include <math.h>
#include <stdint.h>  // Include standard int types
#include <stdio.h>
#include <string.h>

#include "usart.h"

#define GYRO_ADDRESS 0xD0
#define MAG_ADDRESS  0x18  // 磁力计的I2C地址
#define ACCEL_ADDRESS 0xD0

#define Kp 40.0f                       // proportional gain governs rate of convergence to accelerometer/magnetometer
#define Ki 0.02f          // integral gain governs rate of convergence of gyroscope biases
#define halfT 0.005f      // half the sample period
#define dt 0.01f

// Define filter parameters
#define filter_high 0.8f
#define filter_low  0.2f

// extern I2C_HandleTypeDef hi2c2;

// Variables for calibration and filtering
int16_t gyro_offsetx = 0, gyro_offsety = 0, gyro_offsetz = 0;
int16_t accoldx = 0, accoldy = 0, accoldz = 0;
int16_t magoldx = 0, magoldy = 0, magoldz = 0;
int16_t gyrooldx = 0, gyrooldy = 0, gyrooldz = 0;

// Variables for quaternion calculations
float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
float exInt = 0, eyInt = 0, ezInt = 0;
float k10 = 0.0f, k11 = 0.0f, k12 = 0.0f, k13 = 0.0f;
float k20 = 0.0f, k21 = 0.0f, k22 = 0.0f, k23 = 0.0f;
float k30 = 0.0f, k31 = 0.0f, k32 = 0.0f, k33 = 0.0f;
float k40 = 0.0f, k41 = 0.0f, k42 = 0.0f, k43 = 0.0f;

static HAL_StatusTypeDef I2C_WriteByte(uint16_t DevAddress, uint8_t RegAddress, uint8_t Data) {
    uint8_t buffer[2] = {RegAddress, Data};
    return HAL_I2C_Master_Transmit(&hi2c2, DevAddress, buffer, 2, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef I2C_ReadByte(uint16_t DevAddress, uint8_t RegAddress, uint8_t* Data) {
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(&hi2c2, DevAddress, &RegAddress, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    return HAL_I2C_Master_Receive(&hi2c2, DevAddress, Data, 1, HAL_MAX_DELAY);
}

void MPU9250_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIOB clock
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // I2C SCL (PB6) and SDA (PB7) pin configuration
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Enable I2C1 clock
    __HAL_RCC_I2C2_CLK_ENABLE();
}

void MPU9250_I2C_Init(void) {
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 400000; // 400kHz I2C speed
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
}

void MPU9250_Init(void) {
    MPU9250_GPIO_Init();
    MPU9250_I2C_Init();

    I2C_WriteByte(GYRO_ADDRESS, PWR_MGMT_1, 0x00);  // 解除休眠状态
    I2C_WriteByte(GYRO_ADDRESS, SMPLRT_DIV, 0x07);
    I2C_WriteByte(GYRO_ADDRESS, CONFIG, 0x06);
    I2C_WriteByte(GYRO_ADDRESS, GYRO_CONFIG, 0x18);
    I2C_WriteByte(GYRO_ADDRESS, ACCEL_CONFIG, 0x01);
    I2C_WriteByte(GYRO_ADDRESS, 0x37, 0x02);

    // uint8_t mag_id = I2C_ReadByte(MAG_ADDRESS, 0x00, &mag_id);
    // char msg[128];
    // snprintf(msg, sizeof(msg), "ID: 0x%02X\n", mag_id);
    // HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // 初始化磁力计
    I2C_WriteByte(MAG_ADDRESS, 0x0A, 0x11);  // 设置磁力计为单次测量模式2

    // 校准陀螺仪
    calibrate();
}

void calibrate(void) {
    uint8_t t;
    int16_t gx, gy, gz, sumx = 0, sumy = 0, sumz = 0;
    for (t = 0; t < 10; t++) {
        MPU_Get_Gyroscope(&gx, &gy, &gz);
        sumx = sumx + gx;
        sumy = sumy + gy;
        sumz = sumz + gz;
    }
    gyro_offsetx = -sumx / 10;
    gyro_offsety = -sumy / 10;
    gyro_offsetz = -sumz / 10;
}

float invSqrt(float number) {
    long i;
    float x, y;
    const float f = 1.5f;

    x = number * 0.5f;
    y = number;
    i = *((long*)&y);
    i = 0x5f375a86 - (i >> 1);
    y = *((float*)&i);
    y = y * (f - (x * y * y));
    return y;
}

uint8_t MPU_Get_Gyroscope(int16_t *gy, int16_t *gx, int16_t *gz) {
    uint8_t buf[6], res;
    res = I2C_ReadByte(GYRO_ADDRESS, GYRO_XOUT_H, &buf[0]);
    res += I2C_ReadByte(GYRO_ADDRESS, GYRO_XOUT_L, &buf[1]);
    res += I2C_ReadByte(GYRO_ADDRESS, GYRO_YOUT_H, &buf[2]);
    res += I2C_ReadByte(GYRO_ADDRESS, GYRO_YOUT_L, &buf[3]);
    res += I2C_ReadByte(GYRO_ADDRESS, GYRO_ZOUT_H, &buf[4]);
    res += I2C_ReadByte(GYRO_ADDRESS, GYRO_ZOUT_L, &buf[5]);

    if(res == 0) {
        *gy = (((uint16_t)buf[0] << 8) | buf[1]) + gyro_offsety;
        *gx = (((uint16_t)buf[2] << 8) | buf[3]) + gyro_offsetx;
        *gz = (((uint16_t)buf[4] << 8) | buf[5]) + gyro_offsetz;
        *gx = -*gx;
        *gx = (int16_t)(gyrooldx * 0.5 + *gx * 0.5);
        *gy = (int16_t)(gyrooldy * 0.5 + *gy * 0.5);
        *gz = (int16_t)(gyrooldz * 0.5 + *gz * 0.5);
        gyrooldx = *gx;
        gyrooldy = *gy;
        gyrooldz = *gz;
    }
    return res;
}

uint8_t MPU_Get_Accelerometer(int16_t *ay, int16_t *ax, int16_t *az) {
    uint8_t buf[6], res;
    res = I2C_ReadByte(ACCEL_ADDRESS, ACCEL_XOUT_H, &buf[0]);
    res += I2C_ReadByte(ACCEL_ADDRESS, ACCEL_XOUT_L, &buf[1]);
    res += I2C_ReadByte(ACCEL_ADDRESS, ACCEL_YOUT_H, &buf[2]);
    res += I2C_ReadByte(ACCEL_ADDRESS, ACCEL_YOUT_L, &buf[3]);
    res += I2C_ReadByte(ACCEL_ADDRESS, ACCEL_ZOUT_H, &buf[4]);
    res += I2C_ReadByte(ACCEL_ADDRESS, ACCEL_ZOUT_L, &buf[5]);

    if(res == 0) {
        *ay = (((uint16_t)buf[0] << 8) | buf[1]);
        *ax = (((uint16_t)buf[2] << 8) | buf[3]);
        *az = (((uint16_t)buf[4] << 8) | buf[5]);
        *ax = -*ax;
        *ax = (int16_t)(accoldx * filter_high + *ax * filter_low);
        *ay = (int16_t)(accoldy * filter_high + *ay * filter_low);
        *az = (int16_t)(accoldz * filter_high + *az * filter_low);
        accoldx = *ax;
        accoldy = *ay;
        accoldz = *az;
    }
    return res;
}

uint8_t MPU_Get_Magnetometer(int16_t *mx, int16_t *my, int16_t *mz) {
    uint8_t buf[6], res;
    res = I2C_ReadByte(MAG_ADDRESS, 0x03, &buf[0]);
    res += I2C_ReadByte(MAG_ADDRESS, 0x04, &buf[1]);
    res += I2C_ReadByte(MAG_ADDRESS, 0x05, &buf[2]);
    res += I2C_ReadByte(MAG_ADDRESS, 0x06, &buf[3]);
    res += I2C_ReadByte(MAG_ADDRESS, 0x07, &buf[4]);
    res += I2C_ReadByte(MAG_ADDRESS, 0x08, &buf[5]);

    if(res == 0) {
        *mx = ((uint16_t)buf[1] << 8) | buf[0];
        *my = ((uint16_t)buf[3] << 8) | buf[2];
        *mz = ((uint16_t)buf[5] << 8) | buf[4];
        *my = -*my;
        *mz = -*mz;
        *mx = (int16_t)(magoldx * 0.5 + *mx * 0.5);
        *my = (int16_t)(magoldy * 0.5 + *my * 0.5);
        *mz = (int16_t)(magoldz * 0.5 + *mz * 0.5);
        magoldx = *mx;
        magoldy = *my;
        magoldz = *mz;
    }

    I2C_WriteByte(MAG_ADDRESS, 0x0A, 0x11); // Set MAG for continuous measurement mode 2
    return res;
}

uint8_t MPU_Get_Gyro(int16_t *igx, int16_t *igy, int16_t *igz, float *gx, float *gy, float *gz) {
    uint8_t res;
    res = MPU_Get_Gyroscope(igx, igy, igz);
    if (res == 0) {
        *gx = (float)(*igx) / 16.4f;
        *gy = (float)(*igy) / 16.4f;
        *gz = (float)(*igz) / 16.4f;
    }
    return res;
}

uint8_t MPU_Get_Accel(int16_t *iax, int16_t *iay, int16_t *iaz, float *ax, float *ay, float *az) {
    uint8_t res;
    res = MPU_Get_Accelerometer(iax, iay, iaz);
    if (res == 0) {
        *ax = (float)(*iax) / 16384.0f;
        *ay = (float)(*iay) / 16384.0f;
        *az = (float)(*iaz) / 16384.0f;
    }
    return res;
}

uint8_t MPU_Get_Mag(int16_t *imx, int16_t *imy, int16_t *imz, float *mx, float *my, float *mz) {
    uint8_t res;
    res = MPU_Get_Magnetometer(imx, imy, imz);
    if (res == 0) {
        *mx = (float)(*imx) * 0.15f;
        *my = (float)(*imy) * 0.15f;
        *mz = (float)(*imz) * 0.15f;
    }
    return res;
}

// Quaternion update function
void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float *roll, float *pitch, float *yaw) {

           float norm;									//用于单位化
           float hx, hy, hz, bx, bz;		//
           float vx, vy, vz, wx, wy, wz;
           float ex, ey, ez;
//					 float tmp0,tmp1,tmp2,tmp3;

           // auxiliary variables to reduce number of repeated operations  辅助变量减少重复操作次数
           float q0q0 = q0*q0;
           float q0q1 = q0*q1;
           float q0q2 = q0*q2;
           float q0q3 = q0*q3;
           float q1q1 = q1*q1;
           float q1q2 = q1*q2;
           float q1q3 = q1*q3;
           float q2q2 = q2*q2;
           float q2q3 = q2*q3;
           float q3q3 = q3*q3;

           // normalise the measurements  对加速度计和磁力计数据进行规范化
           norm = invSqrt(ax*ax + ay*ay + az*az);
           ax = ax * norm;
           ay = ay * norm;
           az = az * norm;
           norm = invSqrt(mx*mx + my*my + mz*mz);
           mx = mx * norm;
           my = my * norm;
           mz = mz * norm;

           // compute reference direction of magnetic field  计算磁场的参考方向
					 //hx,hy,hz是mx,my,mz在参考坐标系的表示
           hx = 2*mx*(0.50 - q2q2 - q3q3) + 2*my*(q1q2 - q0q3) + 2*mz*(q1q3 + q0q2);
           hy = 2*mx*(q1q2 + q0q3) + 2*my*(0.50 - q1q1 - q3q3) + 2*mz*(q2q3 - q0q1);
           hz = 2*mx*(q1q3 - q0q2) + 2*my*(q2q3 + q0q1) + 2*mz*(0.50 - q1q1 -q2q2);
						//bx,by,bz是地球磁场在参考坐标系的表示
           bx = sqrt((hx*hx) + (hy*hy));
           bz = hz;

// estimated direction of gravity and magnetic field (v and w)  //估计重力和磁场的方向
//vx,vy,vz是重力加速度在物体坐标系的表示
           vx = 2*(q1q3 - q0q2);
           vy = 2*(q0q1 + q2q3);
           vz = q0q0 - q1q1 - q2q2 + q3q3;
					 //wx,wy,wz是地磁场在物体坐标系的表示
           wx = 2*bx*(0.5 - q2q2 - q3q3) + 2*bz*(q1q3 - q0q2);
           wy = 2*bx*(q1q2 - q0q3) + 2*bz*(q0q1 + q2q3);
           wz = 2*bx*(q0q2 + q1q3) + 2*bz*(0.5 - q1q1 - q2q2);

// error is sum ofcross product between reference direction of fields and directionmeasured by sensors
//ex,ey,ez是加速度计与磁力计测量出的方向与实际重力加速度与地磁场方向的误差，误差用叉积来表示，且加速度计与磁力计的权重是一样的
           ex = (ay*vz - az*vy) + (my*wz - mz*wy);
           ey = (az*vx - ax*vz) + (mz*wx - mx*wz);
           ez = (ax*vy - ay*vx) + (mx*wy - my*wx);

           // integral error scaled integral gain
					 //积分误差
           exInt = exInt + ex*Ki*dt;
           eyInt = eyInt + ey*Ki*dt;
           ezInt = ezInt + ez*Ki*dt;
					// printf("exInt=%0.1f eyInt=%0.1f ezInt=%0.1f ",exInt,eyInt,ezInt);
           // adjusted gyroscope measurements
					 //PI调节陀螺仪数据
           gx = gx + Kp*ex + exInt;
           gy = gy + Kp*ey + eyInt;
           gz = gz + Kp*ez + ezInt;
					 //printf("gx=%0.1f gy=%0.1f gz=%0.1f",gx,gy,gz);

           // integrate quaernion rate aafnd normalaizle
					 //欧拉法解微分方程
//           tmp0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
//           tmp1 = q1 + ( q0*gx + q2*gz - q3*gy)*halfT;
//           tmp2 = q2 + ( q0*gy - q1*gz + q3*gx)*halfT;
//           tmp3 = q3 + ( q0*gz + q1*gy - q2*gx)*halfT;
//					 q0=tmp0;
//					 q1=tmp1;
//					 q2=tmp2;
//					 q3=tmp3;
					 //printf("q0=%0.1f q1=%0.1f q2=%0.1f q3=%0.1f",q0,q1,q2,q3);
////RUNGE_KUTTA 法解微分方程
					  k10=0.5 * (-gx*q1 - gy*q2 - gz*q3);
						k11=0.5 * ( gx*q0 + gz*q2 - gy*q3);
						k12=0.5 * ( gy*q0 - gz*q1 + gx*q3);
						k13=0.5 * ( gz*q0 + gy*q1 - gx*q2);

						k20=0.5 * (halfT*(q0+halfT*k10) + (halfT-gx)*(q1+halfT*k11) + (halfT-gy)*(q2+halfT*k12) + (halfT-gz)*(q3+halfT*k13));
						k21=0.5 * ((halfT+gx)*(q0+halfT*k10) + halfT*(q1+halfT*k11) + (halfT+gz)*(q2+halfT*k12) + (halfT-gy)*(q3+halfT*k13));
						k22=0.5 * ((halfT+gy)*(q0+halfT*k10) + (halfT-gz)*(q1+halfT*k11) + halfT*(q2+halfT*k12) + (halfT+gx)*(q3+halfT*k13));
						k23=0.5 * ((halfT+gz)*(q0+halfT*k10) + (halfT+gy)*(q1+halfT*k11) + (halfT-gx)*(q2+halfT*k12) + halfT*(q3+halfT*k13));

						k30=0.5 * (halfT*(q0+halfT*k20) + (halfT-gx)*(q1+halfT*k21) + (halfT-gy)*(q2+halfT*k22) + (halfT-gz)*(q3+halfT*k23));
						k31=0.5 * ((halfT+gx)*(q0+halfT*k20) + halfT*(q1+halfT*k21) + (halfT+gz)*(q2+halfT*k22) + (halfT-gy)*(q3+halfT*k23));
						k32=0.5 * ((halfT+gy)*(q0+halfT*k20) + (halfT-gz)*(q1+halfT*k21) + halfT*(q2+halfT*k22) + (halfT+gx)*(q3+halfT*k23));
						k33=0.5 * ((halfT+gz)*(q0+halfT*k20) + (halfT+gy)*(q1+halfT*k21) + (halfT-gx)*(q2+halfT*k22) + halfT*(q3+halfT*k23));

						k40=0.5 * (dt*(q0+dt*k30) + (dt-gx)*(q1+dt*k31) + (dt-gy)*(q2+dt*k32) + (dt-gz)*(q3+dt*k33));
						k41=0.5 * ((dt+gx)*(q0+dt*k30) + dt*(q1+dt*k31) + (dt+gz)*(q2+dt*k32) + (dt-gy)*(q3+dt*k33));
						k42=0.5 * ((dt+gy)*(q0+dt*k30) + (dt-gz)*(q1+dt*k31) + dt*(q2+dt*k32) + (dt+gx)*(q3+dt*k33));
						k43=0.5 * ((dt+gz)*(q0+dt*k30) + (dt+gy)*(q1+dt*k31) + (dt-gx)*(q2+dt*k32) + dt*(q3+dt*k33));

						q0=q0 + dt/6.0 * (k10+2*k20+2*k30+k40);
						q1=q1 + dt/6.0 * (k11+2*k21+2*k31+k41);
						q2=q2 + dt/6.0 * (k12+2*k22+2*k32+k42);
						q3=q3 + dt/6.0 * (k13+2*k23+2*k33+k43);

           // normalise quaternion
           norm = invSqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
           q0 = q0 * norm;
           q1 = q1 * norm;
           q2 = q2 * norm;
           q3 = q3 * norm;

					 *pitch = asin(-2 * q1 * q3 + 2 * q0 * q2)* 57.3;	// pitch
					 *roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1)* 57.3;	// roll
					 *yaw   = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;	//yaw
           if (*yaw < 0) {
               *yaw += 360.0f;
           }
}
