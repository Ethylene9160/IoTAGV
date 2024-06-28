#ifndef CENTER_MODULE_OSTASK_CONTROLLER_MODULE_PORT_H_
#define CENTER_MODULE_OSTASK_CONTROLLER_MODULE_PORT_H_

#include <queue>

#include "cmsis_os.h"

#include "msgs.h"

#define CTRL_CMD_SET_TWIST 0x0100
#define CTRL_CMD_SET_EXPIRED_TIME 0x0101

namespace ostask_controller_module_port {

    const osThreadAttr_t task_attributes = {
        .name = "osTaskControllerModulePort",
        .stack_size = 256 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument);

    static osMutexId_t controller_command_queue_mutex = nullptr;
    static std::queue<msgs::Command> controller_command_queue;

    bool pushCommand(const msgs::Command &command);
    msgs::Command popCommand();
    bool isCommandQueueEmpty();
    bool clearCommandQueue();

} // namespace ostask_controller_module_port

#endif // CENTER_MODULE_OSTASK_CONTROLLER_MODULE_PORT_H_