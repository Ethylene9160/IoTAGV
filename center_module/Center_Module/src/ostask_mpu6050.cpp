#include "ostask_mpu6050.h"


// QueueHandle_t MPU_Queue;

namespace ostask_mpu6050 {

    [[noreturn]] void taskProcedure(void *argument) {
        float ax, ay, az;
        float gx, gy, gz;
        float alpha;
        char msg[128];

        // 发送任务初始化消息到UART2
        snprintf(msg, sizeof(msg), "Stepping into MPU's ostask\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        while (true) {
            // snprintf(msg, sizeof(msg), "233\r\n");
            // HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            // 读取加速度计数据
            MPU_Read_Accel(&ax, &ay, &az);

            // 读取陀螺仪数据
            MPU_Read_Gyro(&gx, &gy, &gz);

            // 计算航向角
            alpha = MPU_Calculate_Yaw(gz, 0.01f); // 假设时间间隔为0.01s
            snprintf(msg, sizeof(msg), "INTO WHILE alpha %.3f\r\n", alpha);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

            // 将alpha值发送到队列
            // if (xQueueSend(MPU_Queue, &alpha, portMAX_DELAY) != pdTRUE) {
            //     snprintf(msg, sizeof(msg), "Queue send failed\r\n");
            //     HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            // }

            // 打印读取到的数据
            // snprintf(msg, sizeof(msg), "Accel: ax=%.2f, ay=%.2f, az=%.2f; Gyro: gx=%.2f, gy=%.2f, gz=%.2f\r\n", ax, ay, az, gx, gy, gz);
            // HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

            // 延时
            osDelay(100); // 延时50ms，模拟采样间隔
        }
    }

}
