#include "ostask_mpu6050.h"
#include "mpu.h"
#include "kalman.h"
#include "usart.h"
#include "cmsis_os.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "filter.h"

#define PI 3.14159f

namespace ostask_mpu6050 {

    Kalman kalmanYaw;
    center_filter::AverageFilter yawFilter(20, 0.0f);

    float normalizeAngle(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    [[noreturn]] void taskProcedure(void *argument) {
        float ax, ay, az;
        float gx, gy, gz;
        float yaw;
        char msg[128];

        Kalman_Init(&kalmanYaw);

        while (true) {
            // 读取加速度计数据
            MPU_Read_Accel(&ax, &ay, &az);
            MPU_Read_Gyro(&gx, &gy, &gz);

            bool filter_yaw = 0;

            // 计算yaw角
            // yaw = Kalman_Update(&kalmanYaw, std::atan2(ay, az) * 180 / PI, gz, 0.02f);

            // 后来发现yaw无法计算，换成这里将yaw换成row
            yaw = Kalman_Update(&kalmanYaw, std::atan2(ax, az) * 180 / PI, gy, 0.02f);
            yaw = normalizeAngle(yaw);

            if(1 - filter_yaw) {
                snprintf(msg, sizeof(msg), "Calculated yaw %.3f\r\n", yaw);
                HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            }

            if(filter_yaw) {
                // 对yaw值进行滤波
                yawFilter.filter(yaw);
                float filtered_yaw = yawFilter.value();
                snprintf(msg, sizeof(msg), "Filtered yaw: %.3f\r\n", filtered_yaw);
                HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            }

            osDelay(20);
        }
    }
}
