#ifndef OSTASK_VEHICLE_CONTROLLER_H
#define OSTASK_VEHICLE_CONTROLLER_H

#include "cmsis_os.h"

#include "msgs.h"
#include "usart.h"

namespace ostask_vehicle_controller{
    const osThreadAttr_t task_attributes = {
        .name = "osTaskVehicleController",
        .stack_size = 256 << 2,
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument);

    void set_control_msg();


}

#endif //OSTASK_VEHICLE_CONTROLLER_H
