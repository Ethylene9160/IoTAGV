#ifndef OSTASK_MPU6050_H
#define OSTASK_MPU6050_H

#include "cmsis_os.h"
#include "mpu.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

namespace ostask_mpu6050 {

    const osThreadAttr_t task_attributes = {
        .name = "osTaskMPU6050",
        .stack_size = 1024,
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument);

}

#endif // OSTASK_MPU6050_H
