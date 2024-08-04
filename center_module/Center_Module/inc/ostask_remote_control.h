//
// Created by 季吉安 on 2024/8/3.
//

#ifndef CENTER_MODULE_OSTASK_REMOTE_CONTROL_H
#define CENTER_MODULE_OSTASK_REMOTE_CONTROL_H
#include "usart.h"
#include "queue.h"
#include "cmsis_os.h"
#include "port_uart.h"
namespace ostask_remote_control {
    BaseType_t get_xQueueReceive(uint8_t*buffer, TickType_t xTicksToWait) {
        osStatus_t status = osMutexAcquire(USART2_MutexHandle, osWaitForever);
        if (status == osOK) {
            if (xQueueReceive(Remote_Queue, buffer, xTicksToWait) == pdTRUE) {
                osMutexRelease(USART2_MutexHandle);
                return pdTRUE;
            }
        }
        osMutexRelease(USART2_MutexHandle);
        return pdFALSE;
    }

    void read_queue(vehicle_controller* controller) {
        uint8_t buffer[13];
        while(get_xQueueReceive(buffer, 20) == pdTRUE) {
            /**
             * 0: 0x5A
             * 1: msg_type
             * 2: id
             * 3-7: float1
             * 8-11: float2
             * 12: 0x7F
             * total length: 13.
             */
            uint8_t msg_type = buffer[1];
            uint8_t id = buffer[2];
            float f1 = 0.0f, f2 = 0.0f;
            memcpy(&f1, buffer + 3, 4);
            memcpy(&f2, buffer + 8, 4);

            switch(msg_type) {
                // todo: judge id, whether the id maches that of the controller.
                case 0x00: // set position
                    controller->set_target_point({f1, f2});
                    // todo: 发送给上位机
                    break;
                case 0x01:  //velosity?
                    // todo: 发送给上位机
                    break;
                case 0x02: // force stop
                    controller -> stop();
                    break;
                case 0x03: // force start
                    controller -> start();
                    break;
                default:
                    break;
            }
        }
    }
    const osThreadAttr_t task_attributes = {
        .name = "osTaskRemoteControl",
        .stack_size = 256 << 2,
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument) {
        vehicle_controller* controller = (vehicle_controller*)argument;
        // todo.
        osDelay(100);// delay 100 ms.
    }
}

#endif //OSTASK_REMOTE_CONTROL_H
