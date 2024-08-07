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
        while (true) {
            if (!isCommandQueueEmpty()) {
                auto command = popCommand();
                auto v = *(msgs::Twist2D*)command.data_ptr_;
                msgs::Packet packet = msgs::Packet(0xF0, 0x00, 0x00, 0x00, seq_num ++, command);
                msgs::serials packet_serialized = packet.serialize();
                PortCAN::sendBytes(packet_serialized.data_ptr.get(), packet_serialized.len);
                delete command.data_ptr_;
            }
            osDelay(20);
        }

        osMutexDelete(controller_command_queue_mutex);
    }

} // namespace ostask_controller_module_port