#include "ostask_usart_transformer.h"
namespace ostask_usart_transformer{
inline BaseType_t get_xQueueReceiveByte(uint8_t*pdata, uint32_t timeout){
    osStatus_t status = osMutexAcquire(USART_MutexHandle, osWaitForever);
    if (status == osOK) {
        BaseType_t flag = xQueueReceive(S_Queue, pdata, timeout);
        osMutexRelease(USART_MutexHandle);
        return flag;
    }
    return pdFALSE;
}

void taskProcedure(void *argument){
    PortUART *port = (PortUART *)argument;
    uint8_t buffer[255];
    while (1){
        read_from_usart_queue(buffer, 255, port, 10);
        osDelay(10);
    }
}

void read_from_usart_queue(uint8_t *buffer, uint8_t buffer_size, PortUART *port, uint32_t timeout){
    uint8_t buffer_index = 0;
    uint8_t data;
    while (get_xQueueReceiveByte(&data, timeout) == pdTRUE){
        buffer[buffer_index] = data;
        buffer_index++;
        if (buffer_index >= buffer_size){
            // 转变为msgs::uwb_data数据类型变量，推入port。
            msgs::uwb_data _uwb_data(buffer);
            port->pushUWBData(_uwb_data);
        }
    }
}
}