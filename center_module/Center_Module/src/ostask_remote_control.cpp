#include "ostask_remote_control.h"
namespace ostask_remote_control {
    BaseType_t get_xQueueReceive(uint8_t*buffer, TickType_t xTicksToWait) {
        osStatus_t status = osMutexAcquire(USART2_MutexHandle, osWaitForever);
        if (status == osOK) {
            if (xQueueReceive(Remote_Queue, buffer, xTicksToWait) == pdTRUE) {
                osMutexRelease(USART2_MutexHandle);
                return pdTRUE;
            }
        }
        osMutexRelease(USART2_MutexHandle);
        return pdFALSE;
    }

    void read_queue(vehicle_controller* controller) {
        uint8_t buffer[13];

        while(get_xQueueReceive(buffer, 20) == pdTRUE) {
            /**
             * 0: 0x5A
             * 1: msg_type
             * 2: id
             * 3-7: float1
             * 8-11: float2
             * 12: 0x7F
             * total length: 13.
             */
            uint8_t msg_type = buffer[1];
            uint8_t id = buffer[2];
            float f1 = 0.0f, f2 = 0.0f;
            memcpy(&f1, buffer + 3, 4);
            memcpy(&f2, buffer + 8, 4);

            switch(msg_type) {
                // todo: judge id, whether the id maches that of the controller.
                case 0: {
                    // set position
                    controller->set_target_point({f1, f2});
                    char str[32];
                    int len = sprintf(str, "id: %d, self if: %d, set position.\r\n", id, uint8_t(controller->get_self_id()&0xFF));
                    HAL_UART_Transmit(&huart2, (uint8_t*)str, len, 0xffffffff);
                    // todo: 发送给上位机
                    break;
                }

                case 1: {
                    //velosity?
                    // todo: 发送给上位机
                    // char str2[32];
                    // int len2 = sprintf(str2, "set position.\r\n");
                    // HAL_UART_Transmit(&huart2, (uint8_t*)str2, len2, 0xffffffff);
                    break;
                }

                case 2: {
                    // force stop
                    controller -> stop();
                    break;
                }
                case 3: {
                    // force start
                    controller -> start();
                    break;
                }
                default:
                    break;
            }
        }
    }

    [[noreturn]] void taskProcedure(void *argument) {
        vehicle_controller* controller = (vehicle_controller*)argument;
        // todo.
        while(1){
            read_queue(controller);
            osDelay(100);// delay 100 ms.
        }
    }
}