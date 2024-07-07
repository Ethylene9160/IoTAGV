#ifndef CENTER_MODULE_OSTASK_USART_TRANSFORMER_H_
#define CENTER_MODULE_OSTASK_USART_TRANSFORMER_H_
#include "usart.h"
#include "queue.h"
#include "cmsis_os.h"
#include "port_uart.h"

// deprecated.
namespace ostask_usart_transformer {
    BaseType_t get_xQueueReceive(uint8_t*data, uint32_t timeout);

    void read_from_usart_queue(uint8_t *buffer, uint8_t buffer_size, PortUART *port, uint32_t timeout);

    const osThreadAttr_t task_attributes = {
        .name = "osTaskUSARTTransformer",
        .stack_size = 256 << 2,
        .priority = (osPriority_t) osPriorityNormal2,
    };

    [[noreturn]] void taskProcedure(void *argument);
}


#endif
