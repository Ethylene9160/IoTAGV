#include "ostask_uwb_module_port.h"

#include <cstdio>

#include "port_uart.h"
#include "uwb_tool.h"

namespace ostask_uwb_module_port{
void taskProcedure(void *argument) {

    PortUART* port_usart = ((PortUART*)argument);
    uint8_t msg[] = {'2','3','3','3'};
    // msgs::uwb_data _uwb_data2 = port_usart->popUWBData();
    // std::vector<uint8_t> data = _uwb_data2.get_data();
    //
    // msgs::uwb_data _uwb_data2(0, 0, 0);
    // _uwb_data2.set_data(1.0f, 1.5f);
    // port_usart->sendBytes(msg, 4);
    // HAL_UART_Transmit(&huart2, msg, 4, HAL_MAX_DELAY);
    // port_usart->sendBytes(_uwb_data2.serialize().data_ptr.get(), _uwb_data2.serialize().len);
    // return;
    while (true) {
        if (!port_usart->isDataQueueEmpty()) {
            msgs::uwb_data _uwb_data = port_usart->popUWBData();
            // msgs::uwb_data _uwb_data(0, 0, 0);

            // std::vector<uint8_t> data = _uwb_data.get_data();

            // float d1 = 0, d2 = 0;

            // memcpy(&d1, &data[8], sizeof(float));
            // memcpy(&d2, &data[12], sizeof(float));

            //todo: 根据余弦定理计算位置。
            // auto p = dis2cart(d1, d2, 0.1f);
            // printf("x: %f, y: %f\n", p.x, p.second);
            // _uwb_data.set_data(p.x, p.y);
            // uint8_t distance_string[64];
            //
            // sprintf((char*)distance_string, "x: %f, y: %f\n", d1, d2);
            // uint16_t len = strlen((char*)distance_string);

            // _uwb_data.set_data(1.0f, 1.5f);
            // msgs::serials data_serialized = _uwb_data.serialize();
            uint8_t bfe[] = "yyds!\n";
            HAL_UART_Transmit(&huart2, bfe, sizeof(bfe)-1, HAL_MAX_DELAY);
            // port_usart->sendBytes(bfe, 4);//todo: rewrite the data_serilized.
            // uint8_t bfe[] = "yyds!\n";
            // uint8_t b2333[9];
            // memcpy(b2333, &p.x, 4);
            // memcpy(b2333+4, &p.y, 4);
            // b2333[8] = '\n';
            // HAL_UART_Transmit(&huart2, b2333, 9, HAL_MAX_DELAY);
            // HAL_UART_Transmit(&huart2, distance_string, len, HAL_MAX_DELAY);

        }else{
            // uint8_t bfe[] = "Empty!\n";
            // HAL_UART_Transmit(&huart2, bfe, sizeof(bfe)-1, HAL_MAX_DELAY);
        }
        osDelay(30);
    }
    port_usart->release_mutex();
}
}