#include "ostask_uwb_module_port.h"

#include "port_uart.h"

namespace ostask_uwb_module_port{
void taskProcedure(void *argument) {

    PortUART port_usart = *((PortUART*)argument);

    while (true) {
        if (!port_usart.isDataQueueEmpty()) {
            msgs::uwb_data data = port_usart.popUWBData();
            msgs::serials data_serialized = data.serialize();
            port_usart.sendBytes(data_serialized.data_ptr.get(), data_serialized.len);//todo: rewrite the data_serilized.
        }
        osDelay(10);
    }
    port_usart.release_mutex();

}
}