#ifndef CENTER_MODULE_OSTASK_VEHICLE_CONTROLLER_H_
#define CENTER_MODULE_OSTASK_VEHICLE_CONTROLLER_H_

#include "cmsis_os.h"

#include "msgs.h"
#include "usart.h"
#include "vehicle_manager.h"

namespace ostask_vehicle_controller{

    BaseType_t get_xQueueReceive(uint8_t*buffer, TickType_t xTicksToWait);

    const osThreadAttr_t task_attributes = {
        .name = "osTaskVehicleController",
        .stack_size = 256 << 2,
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument);

    void set_control_msg(vehicle_controller* controller);

    void read_queue(vehicle_controller* controller);
}

#endif //OSTASK_VEHICLE_CONTROLLER_H
