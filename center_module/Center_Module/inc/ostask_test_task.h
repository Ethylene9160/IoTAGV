#ifndef OSTASK_TEST_TASK_H
#define OSTASK_TEST_TASK_H
#include <cstdlib>

#include "usart.h"
#include "queue.h"
#include "cmsis_os.h"
#include "port_uart.h"
#include "usart.h"
namespace ostask_test_task{

    void set_test_buffer(uint8_t *buffer, uint16_t self_id, float x, float y) {
        buffer[0] = 0x0A;
        buffer[1] = self_id & 0xFF;
        buffer[2] = (self_id >> 8) & 0xFF;

        buffer[3] = 0;
        buffer[4] = 0;
        buffer[5] = 0;
        buffer[6] = 0;
        buffer[7] = 0;

        memcpy(&buffer[8], &x, 4);
        memcpy(&buffer[12], &y, 4);

        buffer[16] = 0;
        buffer[17] = 0;
        buffer[18] = 0;
        buffer[19] = 0;
        buffer[20] = 0;
        buffer[21] = 0;
        buffer[22] = 0;
        buffer[23] = 0;
        buffer[24] = 0;
        buffer[25] = 0;
        buffer[26] = 0x7F;
    }

    const osThreadAttr_t task_attributes = {
        .name = "osTestTask",
        .stack_size = 256 << 1,
        .priority = (osPriority_t) osPriorityNormal1,
    };

    [[noreturn]] void taskProcedure(void *argument){
        uint8_t buffer[27];
        uint16_t self_id = 1;
        // 将self_id设置为1-3的随机数


        while(1){
            self_id = rand() % 3 + 1;
            float x = 0.0f, y = 0.0f;
            //将x,y设置为3-5之间的随机浮点数
            x = 0.1f + float(rand() % 50) / 100.0f;
            y = 0.1f + float(rand() % 50) / 100.0f;
            set_test_buffer(buffer, self_id, x, y);
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            osStatus_t status = osMutexAcquire(USART1_MutexHandle, osWaitForever);
            if (status != osOK) {
                osMutexRelease(USART1_MutexHandle);
                // HAL_UART_Transmit(&huart2, (uint8_t*)"G1!!!\n", 6, HAL_MAX_DELAY);
                osDelay(2000);
                continue;
            }
            if (xQueueSendFromISR(S_Queue, buffer, &xHigherPriorityTaskWoken) != pdTRUE) {
                // Error_Handler();
                // HAL_UART_Transmit(&huart2, (uint8_t*)"G2!!!\n", 6, HAL_MAX_DELAY);
            }
            osMutexRelease(USART1_MutexHandle);

            // HAL_UART_Transmit(&huart2, (uint8_t*)"yyds!\n", 6, HAL_MAX_DELAY);
            osDelay(2000);
        }
    }
}


#endif //OSTASK_TEST_TASK_H
