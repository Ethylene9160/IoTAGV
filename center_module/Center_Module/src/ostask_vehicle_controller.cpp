#include "ostask_vehicle_controller.h"

#include "ostask_controller_module_port.h"
#include "vehicle_manager.h"
#include "port_can.h"

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
            osDelay(20);
        }
    }

    inline void set_control_msg(vehicle_controller* controller) {
        controller -> tick();
        cart_velocity v = controller -> get_self_velocity();
        msgs::Twist2D twist(v.vx, v.vy, v.w);
        auto command = msgs::Command(CTRL_CMD_SET_TWIST, twist);
        ostask_controller_module_port::pushCommand(command);
    }

    void read_queue(vehicle_controller* controller) {
        // osStatus_t status = osMutexAcquire(USART1_MutexHandle, osWaitForever);
        uint8_t buffer[27];
        while(get_xQueueReceive(buffer, 20) == pdTRUE) {
            //tag_receive_broad(buffer);
            uint16_t source_id = 0;
            uint16_t target_id = 0;
            memcpy(&source_id, buffer + 1, 2);
            memcpy(&target_id, buffer + 3, 2);

            // check
            if (source_id > 0x0FFF && target_id == 0xFFFF) {
                float x = 0.0f, y = 0.0f;
                memcpy(&x, buffer + 8, 4);
                memcpy(&y, buffer + 12, 4);
                cart_point point = {x, y};
                // todo: push the point to the vehicle_position.
                //
            }
        }
    }
}