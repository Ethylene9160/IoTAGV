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
    uint8_t tx_buffer[19];

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

    void send_msg(uint8_t msg_type, uint8_t id, float f1, float f2) {
        tx_buffer[18] = 0x7F;
        // return;
        tx_buffer[8] = msg_type;
        tx_buffer[9] = id;
        memcpy(tx_buffer + 10, &f1, 4);
        memcpy(tx_buffer + 14, &f2, 4);
        HAL_UART_Transmit(&huart2, tx_buffer, 19, HAL_MAX_DELAY);
    }

    void taskProcedure(void *argument) {
        auto* controller = static_cast<vehicle_controller *>(argument);
        tx_buffer[0] = 0x5A;
        tx_buffer[1] = 0x5A;
        tx_buffer[2] = 0x5A;
        tx_buffer[3] = 0x5A;
        tx_buffer[4] = 0x5A;
        tx_buffer[5] = 0xFF;

        // 小端在前
        tx_buffer[6] = 10;
        tx_buffer[7] = 0;
        while (true) {
            read_queue(controller);
            set_control_msg(controller);
            osDelay(50);
        }
    }

    void set_control_msg(vehicle_controller* controller) {
        // static uint8_t tx_buffer[13] = {0x5A, 0,0,0,0,0,0,0,0,0,0,0,0x7F};
        controller -> tick();
        cart_velocity v = controller -> get_self_velocity();


        // msgs::Twist2D* t = new msgs::Twist2D(v.vx, v.vy, v.w);
        flt_vx = alpha * v.vx + (1 - alpha) * flt_vx;
        flt_vy = alpha * v.vy + (1 - alpha) * flt_vy;

        // HAL_UART_Transmit(&huart2, tx_buffer, 13, HAL_MAX_DELAY);
        send_msg((uint8_t)0x01, (uint8_t)(controller->get_self_id()&0xFF), flt_vx, flt_vy);
        // msgs::Twist2D *t = new msgs::Twist2D(v.vy, -v.vx, v.w);
        msgs::Twist2D *t = new msgs::Twist2D(flt_vy, -flt_vx, v.w);
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


                // nan，那么返回。
                if (std::isnan(x) || std::isnan(y)) {
                    return;
                }

                // HAL_UART_Transmit(&huart2, tx_buffer, 13, HAL_MAX_DELAY);
                send_msg((uint8_t)0x00, (uint8_t)(source_id&0xFF), x, y);
                cart_point point = {x, y};
                controller->push_back(source_id, point);
            }
        }
    }
}