#include "ostask_uwb_module_port.h"

#include <cstdio>

#include "port_uart.h"

// deprecated.
namespace ostask_uwb_module_port{
void taskProcedure(void *argument) {

    PortUART* port_usart = ((PortUART*)argument);
    uint8_t msg[] = {'2','3','3','3'};

    while (true) {
        if (!port_usart->isDataQueueEmpty()) {
            msgs::uwb_data _uwb_data = port_usart->popUWBData();

            uint8_t bfe[] = "yyds!\n";
            HAL_UART_Transmit(&huart2, bfe, sizeof(bfe)-1, HAL_MAX_DELAY);


        }else{
        }
        osDelay(30);
    }
    port_usart->release_mutex();
}
}