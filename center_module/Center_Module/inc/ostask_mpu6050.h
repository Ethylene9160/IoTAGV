#ifndef OSTASK_MPU6050_H
#define OSTASK_MPU6050_H

#include "cmsis_os.h"
#include "mpu6050.h"

namespace ostask_mpu6050 {

    const osThreadAttr_t task_attributes = {
        .name = "osTaskMPU6050",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument);

}

// namespace ostask_mpu6050 {
//
//     const osThreadAttr_t task_attributes = {
//         .name = "osTaskVehicleController",
//         .stack_size = 256 << 4,
//         .priority = (osPriority_t) osPriorityNormal,
//     };
//
//     [[noreturn]] void taskProcedure(void *argument);
//
// }


#endif // OSTASK_MPU6050_H
