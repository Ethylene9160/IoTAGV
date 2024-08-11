#include "ostask_vehicle_controller.h"

#include <cmath>
#include <cstdio>
#include <memory>

#include "queue.h"

#include "ostask_controller_module_port.h"
#include "ostask_remote_control.h"
#include "vehicle_controller.h"

using msgs::send_msg_to_host;


namespace ostask_vehicle_controller {

    float flt_vx = 0.0f, flt_vy = 0.0f;
    float alpha = 0.233f;

    BaseType_t get_xQueueReceive(uint8_t* buffer, TickType_t xTicksToWait) {
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
            controller -> tick(); // 控制算法步进 (TODO 挪了下位置)

            read_queue(controller); // 处理 UWB 发来的报文

            set_control_msg(controller); // 发送底盘控制指令

            osDelay(25);
        }
    }

    void set_control_msg(vehicle_controller* controller) {
        // 获取目标速度与滤波
        cart_velocity v = controller -> get_self_velocity();
        flt_vx = alpha * v.vx + (1 - alpha) * flt_vx;
        flt_vy = alpha * v.vy + (1 - alpha) * flt_vy;
//         msgs::Twist2D* t = new msgs::Twist2D(v.vx, v.vy, v.w);

        // 发送自身目标速度信息给上位机
        send_msg_to_host(RC_DATAGRAM_VELOCITY, controller->get_self_id(), flt_vx, flt_vy);

//         char str[48];
//         snprintf(str, sizeof(str), "\r\ncurrent delta: %.3f, w: %.3f\r\n", controller->get_init_alpha() - controller->get_current_alpha(), v.w);
//         HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);

        msgs::Twist2D *t = new msgs::Twist2D(flt_vy, -flt_vx, v.w); // 负号是坐标系的问题
//        msgs::Twist2D *t = new msgs::Twist2D(flt_vy, -flt_vx, 0.0f);

        // 到 controller_module 的控制指令
        auto command = msgs::Command(CTRL_CMD_SET_TWIST, t);
        ostask_controller_module_port::pushCommand(command);
    }

    // 从 S_Queue 中读取 UWB 发来的报文 (帧内包含位置分享和可能存在的指令转发)
    void read_queue(vehicle_controller* controller) {
        uint8_t buffer[27];
        while (get_xQueueReceive(buffer, 20) == pdTRUE) {
            uint16_t source_id = 0;
            uint16_t target_id = 0;
            memcpy(&source_id, buffer + 1, 2);
            memcpy(&target_id, buffer + 3, 2);
            uint8_t ctrl_type = buffer[5]; // 改为用作消息类型了 TODO 分别是什么

            float x = 0.0f, y = 0.0f, opt1 = 0.0f, opt2 = 0.0f;
            memcpy(&x, buffer + 8, 4);
            memcpy(&y, buffer + 12, 4);
            memcpy(&opt1, buffer + 16, 4);
            memcpy(&opt2, buffer + 20, 4);
//            char str[64];
//            int len = sprintf(str, "\r\nsrc: %d, target: %d, ctrl: %d, x: %.3f, y: %.3f, opt1: %.3f, opt2: %.3f\r\n", source_id, target_id, ctrl_type, x, y, opt1, opt2);
//            HAL_UART_Transmit(&huart2, (uint8_t*) str, len, HAL_MAX_DELAY);

            // 如果存在 NaN 则丢弃
            if (std::isnan(x) || std::isnan(y) || std::isnan(opt1) || std::isnan(opt2)) {
                continue; // TODO
            }

            // 将收到的位置信息发送给上位机 TODO 所有吗
            send_msg_to_host(RC_DATAGRAM_POSITION, uint8_t(source_id & 0xFF), x, y);

            // 将收到的位置信息更新到控制算法中
            cart_point point = {x, y};
            controller->push_back(source_id, point);

            // 处理指令转发
            if (target_id == controller->get_self_id()) {
                controller->process_remote_command(ctrl_type, opt1, opt2);
            }
        }
    }

}