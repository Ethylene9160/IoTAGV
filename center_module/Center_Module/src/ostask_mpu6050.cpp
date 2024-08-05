#include "ostask_mpu6050.h"
#include "mpu.h"
#include "kalman.h"
#include "usart.h"
#include "cmsis_os.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cmath>

#define PI 3.14159f

namespace ostask_mpu6050 {

    Kalman kalmanYaw;

    [[noreturn]] void taskProcedure(void *argument) {
        float ax, ay, az;
        float gx, gy, gz;
        float yaw;
        char msg[128];

        Kalman_Init(&kalmanYaw);

        while (true) {
            // 读取加速度计数据
            MPU_Read_Accel(&ax, &ay, &az);

            // 读取陀螺仪数据
            MPU_Read_Gyro(&gx, &gy, &gz);

            // 计算航向角
            yaw = Kalman_Update(&kalmanYaw, std::atan2(ay, ax) * 180 / PI, gz, 0.1f); // 使用 Kalman 滤波器，假设时间间隔为0.01s
            snprintf(msg, sizeof(msg), "INTO WHILE yaw %.3f\r\n", yaw);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

            // 将yaw值发送到队列
            // if (xQueueSend(MPU_Queue, &yaw, portMAX_DELAY) != pdTRUE) {
            //     snprintf(msg, sizeof(msg), "Queue send failed\r\n");
            //     HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            // }

            // 延时
            osDelay(100); // 延时100ms，模拟采样间隔
        }
    }

}
