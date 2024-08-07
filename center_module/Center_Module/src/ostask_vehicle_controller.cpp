#include "ostask_vehicle_controller.h"

#include <cmath>
#include <cstdio>

#include "ostask_controller_module_port.h"
#include "vehicle_controller.h"

#include "queue.h"
#include "mpu.h"
#include <memory>

using msgs::send_msg_to_host;

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
//        HAL_UART_Transmit(&huart2, (uint8_t*)"233\r\n", 5, 0xffffffff);
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
        // send_msg((uint8_t)0x01, (uint8_t)(controller->get_self_id()&0xFF), flt_vx, flt_vy);
        // send velosity to upper.
        send_msg_to_host(VELOCITY_CTRL, controller->get_self_id(), flt_vx, flt_vy);
        // char str[32];
        // snprintf(str, sizeof(str), "vx: %.3f, vy: %.3f\r\n", flt_vx, flt_vy);
        // HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
        // msgs::Twist2D *t = new msgs::Twist2D(v.vy, -v.vx, v.w);
        msgs::Twist2D *t = new msgs::Twist2D(flt_vy, -flt_vx, v.w);
        auto command = msgs::Command(CTRL_CMD_SET_TWIST, t);

        ostask_controller_module_port::pushCommand(command);
    }

    void read_queue(vehicle_controller* controller) {
        uint8_t buffer[27];
        while(get_xQueueReceive(buffer, 20) == pdTRUE) {
            uint16_t source_id = 0;
            uint16_t target_id = 0;
            memcpy(&source_id, buffer + 1, 2);
            memcpy(&target_id, buffer + 3, 2);
            uint8_t ctrl_type = buffer[5];

            float x = 0.0f, y = 0.0f, d1 = 0.0f, d2 = 0.0f;
            memcpy(&x, buffer + 8, 4);
            memcpy(&y, buffer + 12, 4);
            memcpy(&d1, buffer + 16, 4);
            memcpy(&d2, buffer + 20, 4);
            char str[64];
            snprintf(str, sizeof(str), "srcid: %d, tarid: %d, x: %.3f, y: %.3f, d1: %.3f, d2: %.3f\r\n", source_id, target_id, x, y, d1, d2);
            HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
            // nan, return.
            if (std::isnan(x) || std::isnan(y) || std::isnan(d1) || std::isnan(d2)) {
                return;
            }

            // send position to upper.
            send_msg_to_host(POSITION_CTRL, source_id&0xFF, x, y);

            cart_point point = {x, y};
            controller->push_back(source_id, point);
            if (target_id == controller->get_self_id()) {
                switch (ctrl_type) {
                    case POSITION_CTRL:
                        // set target position
                            controller->set_target_point({d1, d2});
                            break;
                    case VELOCITY_CTRL:
                        vehicle_controller::v_cons = d1;
                        break;
                    case STOP_CTRL:
                        controller->stop();
                        break;
                    case START_CTRL:
                        controller->start();
                        break;
                    default:
                        break;
                }
            }

        }
    }
}