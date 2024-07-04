#include "ostask_vehicle_controller.h"

#include "ostask_controller_module_port.h"
#include "vehicle_manager.h"
#include "port_can.h"

namespace ostask_vehicle_controller {
    void taskProcedure(void *argument) {
        // PortCAN *port_can = (PortCAN*)argument;

        while (true) {
            set_control_msg();
            // todo: set the command.
            auto command = msgs::Command(CTRL_CMD_SET_TWIST, msgs::Twist2D(20.0f, 0.0f, 0.0f));
            ostask_controller_module_port::pushCommand(command);
            osDelay(20);
        }
    }

    void set_control_msg() {
        //todo.
        // 遍历哈希表
        for (auto &vehicle : vehicle_position) {
            // todo
        }
    }
}