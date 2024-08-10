#include "ostask_remote_control.h"

using msgs::send_msg_to_uwb;

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
        uint8_t buffer[12];

        while(get_xQueueReceive(buffer, 20) == pdTRUE) {
            /**
             * 0: 0x5A
             * 1: msg_type
             * 2: id
             * 3-6: float1
             * 7-10: float2
             * 11: 0x7F
             * total length: 12.
             */
            uint8_t msg_type = buffer[1];
            uint8_t id = buffer[2];
            float f1 = 0.0f, f2 = 0.0f;
            memcpy(&f1, buffer + 3, 4);
            memcpy(&f2, buffer + 7, 4);

            if (id != controller->get_self_id()) {
                char str[32];
                // HAL_UART_Transmit(&huart2, buffer, 12, HAL_MAX_DELAY);
                snprintf(str, sizeof(str), "id not match: %d, %d, ctrl: %d\r\n", id, controller->get_self_id(), msg_type);
                HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);

                send_msg_to_uwb(msg_type, id, f1, f2);
                // HAL_UART_Transmit(&huart1, buffer, 12, HAL_MAX_DELAY);
                return;
            }
            //相同id，直接设置。
            switch(msg_type) {
                case POSITION_CTRL: {
                    // set position
                    char chs[48];
                    int len = sprintf(chs, "set self p: .2%f, %.2f\r\n", f1, f2);
                    HAL_UART_Transmit(&huart2, (uint8_t*)chs, len, HAL_MAX_DELAY);
                    controller->set_target_point({f1, f2});
                    break;
                }

                case VELOCITY_CTRL: {
                    vehicle_controller::v_cons = f1;
                    break;
                }

                case STOP_CTRL: {
                    // force stop
                    controller -> stop();
                    break;
                }
                case START_CTRL: {
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