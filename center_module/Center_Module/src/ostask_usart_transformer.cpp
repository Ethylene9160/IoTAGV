#include "ostask_usart_transformer.h"
namespace ostask_usart_transformer{
inline BaseType_t get_xQueueReceive(uint8_t*pdata, uint32_t timeout){
    osStatus_t status = osMutexAcquire(USART1_MutexHandle, timeout);
    if (status == osOK) {
        BaseType_t flag = xQueueReceive(S_Queue, pdata, timeout);
        osMutexRelease(USART1_MutexHandle);
        return flag;
    }
    // else {
    //     uint8_t bfr[] = "ERROR IN get_xQueueReceiveByte\n";
    //     HAL_UART_Transmit(&huart2, bfr, sizeof(bfr)-1, HAL_MAX_DELAY);
    // }
    return pdFALSE;
}

void taskProcedure(void *argument){
    PortUART *port = (PortUART *)argument;
    uint8_t buffer_size = 19;
    uint8_t buffer[32]; //实际上只有19位需要使用。
    uint8_t buffer_index = 0;
    uint8_t data;
    while (true){
        // uint8_t* pdata = 0;
        uint8_t bfr_t[] = "read_from_usart_queue\n";

        // HAL_UART_Transmit(&huart2, bfr_t, sizeof(bfr_t)-1, HAL_MAX_DELAY);
        while (get_xQueueReceive(&data, 50) == pdTRUE){
            // while (xQueueReceive(S_Queue, &data, timeout) == pdTRUE){

            buffer[buffer_index] = data;
            buffer_index++;
            // HAL_UART_Transmit(&huart2, bfr_t, sizeof(bfr_t)-1, HAL_MAX_DELAY);

            if (buffer_index >= buffer_size){
                // 转变为msgs::uwb_data数据类型变量，推入port。
                // msgs::uwb_data _uwb_data(0,0,0);
                // _uwb_data.set_data(1.4,1.5);
                uint8_t d[] = {0,0,0,0,0,0,8,0,1,2,3,4,5,6,7,8,0,0,0};
                msgs::uwb_data _uwb_data(d);
                // buffer_index = 0;
                uint8_t bfr[] = "ERROR IN read_from_usart_queue\n";
                if(port->pushUWBData(_uwb_data)) {
                    // HAL_UART_Transmit(&huart2, _uwb_data.serialize().data_ptr.get(), _uwb_data.serialize().len, HAL_MAX_DELAY);
                    HAL_UART_Transmit(&huart2, bfr_t, sizeof(bfr_t)-1, HAL_MAX_DELAY);
                //
                }else {
                    HAL_UART_Transmit(&huart2, bfr, sizeof(bfr)-1, HAL_MAX_DELAY);
                }
            }
        }
        osDelay(50);
    }
}

    // deprecated
void read_from_usart_queue(uint8_t *buffer, uint8_t buffer_size, PortUART *port, uint32_t timeout){
    uint8_t buffer_index = 0;
    uint8_t data;
    // uint8_t* pdata = 0;
    uint8_t bfr_t[] = "read_from_usart_queue\n";

    HAL_UART_Transmit(&huart2, bfr_t, sizeof(bfr_t)-1, HAL_MAX_DELAY);
    while (get_xQueueReceive(&data, timeout) == pdTRUE){
    // while (xQueueReceive(S_Queue, &data, timeout) == pdTRUE){

        buffer[buffer_index] = data;
        buffer_index++;
        // HAL_UART_Transmit(&huart2, bfr_t, sizeof(bfr_t)-1, HAL_MAX_DELAY);

        if (buffer_index >= buffer_size){
            // 转变为msgs::uwb_data数据类型变量，推入port。
            // msgs::uwb_data _uwb_data(0,0,0);

            buffer_index = 0;
            uint8_t bfr[] = "ERROR IN read_from_usart_queue\n";
            // if(port->pushUWBData(_uwb_data)) {
            // HAL_UART_Transmit(&huart2, bfr_t, sizeof(bfr_t)-1, HAL_MAX_DELAY);
            //
            // }else {
            HAL_UART_Transmit(&huart2, bfr, sizeof(bfr)-1, HAL_MAX_DELAY);
            // }
        }
    }
}
}