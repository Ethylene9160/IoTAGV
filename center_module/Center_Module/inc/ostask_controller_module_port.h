#ifndef CENTER_MODULE_OSTASK_CONTROLLER_MODULE_PORT_H_
#define CENTER_MODULE_OSTASK_CONTROLLER_MODULE_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"


const osThreadAttr_t controller_module_port_task_attributes = {
    .name = "controllerModulePortTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};


[[noreturn]] void controllerModulePortTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif