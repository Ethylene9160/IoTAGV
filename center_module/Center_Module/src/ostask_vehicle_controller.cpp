#include "ostask_vehicle_controller.h"

#include <cmath>
#include <cstdio>

#include "ostask_controller_module_port.h"
#include "vehicle_controller.h"

#include "queue.h"
#include <memory>
namespace ostask_vehicle_controller {
    float flt_vx = 0.0f, flt_vy = 0.0f;
    float alpha = 0.233f;
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
        static uint8_t tx_buffer[13] = {0x5A, 0,0,0,0,0,0,0,0,0,0,0,0x7F};
        controller -> tick();
        cart_velocity v = controller -> get_self_velocity();


        // msgs::Twist2D* t = new msgs::Twist2D(v.vx, v.vy, v.w);
        flt_vx = alpha * v.vx + (1 - alpha) * flt_vx;
        flt_vy = alpha * v.vy + (1 - alpha) * flt_vy;

        tx_buffer[1] = (controller->get_self_id()&0xFF);
        tx_buffer[2] = 0x01;
        memcpy(tx_buffer + 3, &flt_vx, 4);
        memcpy(tx_buffer + 7, &flt_vy, 4);
        HAL_UART_Transmit(&huart2, tx_buffer, 13, HAL_MAX_DELAY);

        // msgs::Twist2D *t = new msgs::Twist2D(v.vy, -v.vx, v.w);
        msgs::Twist2D *t = new msgs::Twist2D(flt_vy, -flt_vx, v.w);
        auto command = msgs::Command(CTRL_CMD_SET_TWIST, t);

        ostask_controller_module_port::pushCommand(command);
    }

    void read_queue(vehicle_controller* controller) {
        uint8_t buffer[27];
        static uint8_t tx_buffer[13] = {0x5A, 0,0,0,0,0,0,0,0,0,0,0,0x7F};
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


                // nan，那么返回。
                if (std::isnan(x) || std::isnan(y)) {
                    return;
                }

                tx_buffer[1] = source_id & 0xFF;
                tx_buffer[2] = 0x00; // position信息

                memcpy(tx_buffer + 3, &x, 4);
                memcpy(tx_buffer + 7, &y, 4);

                HAL_UART_Transmit(&huart2, tx_buffer, 13, HAL_MAX_DELAY);

                cart_point point = {x, y};
                controller->push_back(source_id, point);
            }
        }
    }
}