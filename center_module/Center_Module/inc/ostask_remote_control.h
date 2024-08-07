#ifndef CENTER_MODULE_OSTASK_REMOTE_CONTROL_H
#define CENTER_MODULE_OSTASK_REMOTE_CONTROL_H
#include <cstdio>

#include "usart.h"
#include "queue.h"
#include "cmsis_os.h"
#include "port_uart.h"
namespace ostask_remote_control {
    BaseType_t get_xQueueReceive(uint8_t*buffer, TickType_t xTicksToWait);

    void read_queue(vehicle_controller* controller);

    const osThreadAttr_t task_attributes = {
        .name = "osTaskRemoteControl",
        .stack_size = 256 << 2,
        .priority = (osPriority_t) osPriorityBelowNormal7,
    };

    [[noreturn]] void taskProcedure(void *argument);
}

#endif //OSTASK_REMOTE_CONTROL_H
