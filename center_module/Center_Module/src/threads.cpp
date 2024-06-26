#include "main.h"

#include "cmsis_os.h"

#include "ostask_controller_module_port.h"


void startThreads() {
    osThreadId_t controller_module_port_task_handle;
    controller_module_port_task_handle = osThreadNew(controllerModulePortTask, NULL, &controller_module_port_task_attributes);
}
