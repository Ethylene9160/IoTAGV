#include "port_uart.h"

#include <cstring>

#include "cmsis_os.h"

bool PortUART::init(uint16_t module_id, UART_HandleTypeDef &usart) {
    this->_module_id = module_id;
    this->_huart = usart;

    bool state = true;

    return state;
}


