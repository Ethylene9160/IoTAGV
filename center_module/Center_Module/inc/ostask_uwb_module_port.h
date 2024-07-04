#ifndef CENTER_MODULE_OSTASK_UWB_MODULE_PORT_H_
#define CENTER_MODULE_OSTASK_UWB_MODULE_PORT_H_

#include "cmsis_os.h"

#include "msgs.h"

// deprecated.
namespace ostask_uwb_module_port {

    const osThreadAttr_t task_attributes = {
        .name = "osTaskUwbModulePort",
        .stack_size = 256 << 1,
        .priority = (osPriority_t) osPriorityNormal1,
    };

    [[noreturn]] void taskProcedure(void *argument);

} // namespace ostask_controller_module_port

#endif //CENTER_MODULE_OSTASK_UWB_MODULE_PORT_H_

