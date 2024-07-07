#include "ostask_controller_module_port.h"

#include <cstdio>

#include "port_can.h"
#include "usart.h"

msgs::Twist2D NULL_TWIST(0.0f,0.0f,0.0f);

namespace ostask_controller_module_port {

    bool pushCommand(msgs::Command command) {
        if (controller_command_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(controller_command_queue_mutex, osWaitForever);
            if (status == osOK) {
                controller_command_queue.push(command);
                osMutexRelease(controller_command_queue_mutex);
                return true;
            }
        }
        return false;
    }

    msgs::Command popCommand() {
        if (controller_command_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(controller_command_queue_mutex, osWaitForever);
            if (status == osOK) {
                if (!controller_command_queue.empty()) {
                    auto command = controller_command_queue.front();
                    controller_command_queue.pop();
                    osMutexRelease(controller_command_queue_mutex);
                    return command;
                }
            }
        }
        return msgs::Command(0, &NULL_TWIST);
    }

    bool isCommandQueueEmpty() {
        if (controller_command_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(controller_command_queue_mutex, osWaitForever);
            if (status == osOK) {
                bool empty = controller_command_queue.empty();
                osMutexRelease(controller_command_queue_mutex);
                return empty;
            }
        }
        return true; // TODO
    }

    bool clearCommandQueue() {
        if (controller_command_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(controller_command_queue_mutex, osWaitForever);
            if (status == osOK) {
                while (!controller_command_queue.empty()) {
                    controller_command_queue.pop();
                }
                osMutexRelease(controller_command_queue_mutex);
                return true;
            }
        }
        return false;
    }

    void taskProcedure(void *argument) {
        controller_command_queue_mutex = osMutexNew(nullptr);

        PortCAN::init(0x061A);

        uint16_t seq_num = 0;
        char bft[32];
        char bfe[32];
        int len = sprintf(bft, "Controller Module Port Task\n");
        int len_e = sprintf(bfe, "Controller Module Port Task Error\n");
        while (true) {
            if (!isCommandQueueEmpty()) {
                auto command = popCommand();
                auto v = *(msgs::Twist2D*)command.data_ptr_;
                msgs::Packet packet = msgs::Packet(0xF0, 0x00, 0x00, 0x00, seq_num ++, command);
                msgs::serials packet_serialized = packet.serialize();
                PortCAN::sendBytes(packet_serialized.data_ptr.get(), packet_serialized.len);

                char bft233[32];
                int len = sprintf(bft233, "vx: %.2f, vy: %.2f, w: %.2f\n", v.linear_x, v.linear_y, v.angular_z);
                // HAL_UART_Transmit(&huart2, (uint8_t *)packet_serialized.data_ptr.get(), packet_serialized.len, 0xffff);
                HAL_UART_Transmit(&huart2, (uint8_t *)bft233, len, 0xffff);
                delete command.data_ptr_;
            }
            osDelay(20);
        }

        osMutexDelete(controller_command_queue_mutex);
    }

} // namespace ostask_controller_module_port