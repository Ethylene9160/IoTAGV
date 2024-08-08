#ifndef OSTASK_MPU9250_H
#define OSTASK_MPU9250_H

#include "../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os.h"
#include "../../Core/Inc/mpu9250_dmp.h"
#include <stdio.h>
#include <string.h>
#include "../../Core/Inc/usart.h"
#include "../../Middlewares/Third_Party/FreeRTOS/Source/include/FreeRTOS.h"
#include "../../Middlewares/Third_Party/FreeRTOS/Source/include/task.h"
#include "../../Middlewares/Third_Party/FreeRTOS/Source/include/queue.h"

namespace ostask_mpu9250 {

    const osThreadAttr_t task_attributes = {
        .name = "osTaskMPU9250",
        .stack_size = 1024,
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument);


}

extern float pitch_inside, roll_inside, yaw_inside;
#endif // OSTASK_MPU9250_H
