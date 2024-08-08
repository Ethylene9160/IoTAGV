#include "../inc/ostask_mpu9250.h"
#include "../../Core/Inc/mpu9250_dmp.h"
#include "../../Core/Inc/usart.h"
#include "../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "../inc/filter.h"

#define PI 3.14159f

namespace ostask_mpu9250 {

    [[noreturn]] void taskProcedure(void *argument) {
        char msg[128];
        while (true) {
            if (MPU9250_dataReady()) {
                // 读取所有传感器数据
                if (MPU9250_update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS) != INV_SUCCESS) {
                    snprintf(msg, sizeof(msg), "Failed to read sensor data\r\n");
                    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
                }

                // 计算欧拉角
                MPU9250_computeEulerAngles(true);

                snprintf(msg, sizeof(msg), "Calculated yaw %.3f\r\n", yaw_inside);
                HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            }
            osDelay(20);
        }
    }
}
