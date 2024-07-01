#include "ostask_uwb_module_port.h"

#include "port_uart.h"
#include "uwb_tool.h"

namespace ostask_uwb_module_port{
void taskProcedure(void *argument) {

    PortUART port_usart = *((PortUART*)argument);

    while (true) {
        if (!port_usart.isDataQueueEmpty()) {
            msgs::uwb_data _uwb_data = port_usart.popUWBData();
            std::vector<uint8_t> data = _uwb_data.get_data();

            float d1 = 0, d2 = 0;
            memcpy(&d1, &data[0], sizeof(float));
            memcpy(&d2, &data[4], sizeof(float));

            //todo: 根据余弦定理计算位置。
            auto p = dis2cart(d1, d2, 0.1f);

            _uwb_data.set_data(1.0f, 1.5f);
            msgs::serials data_serialized = _uwb_data.serialize();
            port_usart.sendBytes(data_serialized.data_ptr.get(), data_serialized.len);//todo: rewrite the data_serilized.
        }
        osDelay(10);
    }
    port_usart.release_mutex();

}
}