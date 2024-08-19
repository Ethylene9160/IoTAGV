#include "ostask_mpu9250.h"
#include "mpu9250.h"
#include "usart.h"
#include "cmsis_os.h"
#include <cmath>
#include <cstdio>

#define PI 3.14159f

namespace ostask_mpu9250 {

    [[noreturn]] void taskProcedure(void *argument) {
        char msg[128];
        float ax, ay, az, mx, my, mz, roll, pitch, yaw, gx, gy, gz;
        int16_t iax, iay, iaz;
        int16_t imx, imy, imz;
        int16_t igx, igy, igz;

        CalibrateMagnetometerTimed();

        while (true) {
            // 获取并处理加速度计数据
            MPU_Get_Accel(&iax, &iay, &iaz, &ax, &ay, &az);

            // 获取并处理磁力计数据
            MPU_Get_Mag(&imx, &imy, &imz, &mx, &my, &mz);

            // 将磁力计数据进行校准
            ApplyMagCalibration(&imx, &imy, &imz);

            // 获取并处理陀螺仪数据
            MPU_Get_Gyro(&igx, &igy, &igz, &gx, &gy, &gz);

            // 使用获取的数据进行姿态解算
            AHRSupdate(gx, gy, gz, ax, ay, az, mx, my, mz, &roll, &pitch, &yaw);

            // snprintf(msg, sizeof(msg), "imx:%d, imy:%d, imz:%d\r\n", imx, imy, imz);
            // snprintf(msg, sizeof(msg), "gx:%.3f, gy:%3f, gz:%3f\r\n", gx, gy, gz);
            // snprintf(msg, sizeof(msg), "ax:%.3f, ay:%3f, az:%3f\r\n", ax, ay, az);

            // 将解算出的Yaw角度通过串口发送
            snprintf(msg, sizeof(msg), "Yaw: %.3f\r\n", yaw);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

            // 延时10毫秒
            osDelay(10);
        }
    }
}
