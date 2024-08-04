#include "ostask_mpu6050.h"
#include "mpu6050.h"
#include <stdio.h>
#include <string.h>

#include "usart.h"

namespace ostask_mpu6050 {

    [[noreturn]] void taskProcedure(void *argument) {
        float alpha;
        char msg[128];

        // Send task initialized message to UART2
        snprintf(msg, sizeof(msg), "Stepping into MPU's ostask\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        while (true) {
            snprintf(msg, sizeof(msg), "INTO THE WHILE FUNCTION\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

            if (xQueueReceive(MPU_Queue, &alpha, portMAX_DELAY) == pdPASS) {
                snprintf(msg, sizeof(msg), "Yaw angle: %.2f\r\n", alpha);
                HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            }

            snprintf(msg, sizeof(msg), "OUT OF WHILE FUNCTION\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

            osDelay(50);
        }
    }

}
// namespace ostask_mpu6050 {
//
//     [[noreturn]] void taskProcedure(void *argument) {
//         float alpha;
//         char msg[128];
//
//         // Send task initialized message to UART2
//         snprintf(msg, sizeof(msg), "Stepping into MPU's ostask\r\n");
//         HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
//
//         while (true) {
//             snprintf(msg, sizeof(msg), "INTO THE WHILE FUNCTION\r\n");
//             HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
//             if (xQueueReceive(MPU_Queue, &alpha, portMAX_DELAY) == pdPASS) {
//                 snprintf(msg, sizeof(msg), "Yaw angle: %.2f\r\n", alpha);
//                 HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
//             }
//             snprintf(msg, sizeof(msg), "OUT OF WHILE FUNCTION\r\n");
//             HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
//             osDelay(50);
//         }
//     }
//
// }
