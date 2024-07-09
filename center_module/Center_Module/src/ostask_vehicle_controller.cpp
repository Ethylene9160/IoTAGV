#include "ostask_vehicle_controller.h"

#include <cmath>
#include <cstdio>

#include "ostask_controller_module_port.h"
#include "vehicle_controller.h"

#include "queue.h"
#include <memory>
namespace ostask_vehicle_controller {
    BaseType_t get_xQueueReceive(uint8_t*buffer, TickType_t xTicksToWait) {
        osStatus_t status = osMutexAcquire(USART1_MutexHandle, osWaitForever);
        if (status == osOK) {
            if (xQueueReceive(S_Queue, buffer, xTicksToWait) == pdTRUE) {
                osMutexRelease(USART1_MutexHandle);
                return pdTRUE;
            }
        }
        osMutexRelease(USART1_MutexHandle);
        return pdFALSE;
    }

    void taskProcedure(void *argument) {
        auto* controller = static_cast<vehicle_controller *>(argument);
        while (true) {
            read_queue(controller);
            set_control_msg(controller);
            osDelay(50);
        }
    }

    void set_control_msg(vehicle_controller* controller) {
        controller -> tick();
        cart_velocity v = controller -> get_self_velocity();


        char str[32];
        int len = sprintf(str, "1vx: %d, vy: %d\n", (int)(v.vx * 1200+50001), (int)(v.vy * 1200+50001));
        HAL_UART_Transmit(&huart2, (uint8_t*)str, len, HAL_MAX_DELAY);

        // msgs::Twist2D* t = new msgs::Twist2D(v.vx, v.vy, v.w);

        msgs::Twist2D *t = new msgs::Twist2D(v.vy, -v.vx, v.w);
        auto command = msgs::Command(CTRL_CMD_SET_TWIST, t);

        ostask_controller_module_port::pushCommand(command);
    }

    void read_queue(vehicle_controller* controller) {
        uint8_t buffer[27];
        while(get_xQueueReceive(buffer, 20) == pdTRUE) {
            //tag_receive_broad(buffer);
            uint16_t source_id = 0;
            uint16_t target_id = 0;
            memcpy(&source_id, buffer + 1, 2);
            memcpy(&target_id, buffer + 3, 2);

            // if (source_id > 0x0FFF && target_id == 0xFFFF) {
            if (1){
                float x = 0.0f, y = 0.0f, d1 = 0.0f, d2 = 0.0f;
                memcpy(&x, buffer + 8, 4);
                memcpy(&y, buffer + 12, 4);
                memcpy(&d1, buffer + 16, 4);
                memcpy(&d2, buffer + 20, 4);
                char str[64];
                // // int len = sprintf(str, "source_id: %d, x: %f, y: %f\n", source_id, x, y);
                int len = sprintf(str, "0id: %d, x: %d, y: %d, d1: %d, d2: %d\n",
                    source_id,
                    (int) (x * 10000),
                    (int) (y * 10000),
                    (int) (d1 * 10000),
                    (int) (d2 * 10000));
                HAL_UART_Transmit(&huart2, (uint8_t*)str, len, HAL_MAX_DELAY);

                // nan，那么返回。
                if (std::isnan(x) || std::isnan(y)) {
                    return;
                }
                cart_point point = {x, y};
                controller->push_back(source_id, point);
            }
        }
    }
}