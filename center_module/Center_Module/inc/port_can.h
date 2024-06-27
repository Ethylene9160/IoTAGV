#ifndef CENTER_MODULE_PORT_CAN_H_
#define CENTER_MODULE_PORT_CAN_H_

#include <cstdint>

#include "can.h"

class PortCAN {
public:
    static bool init(uint16_t module_id, CAN_HandleTypeDef &hcan = hcan1);
    static bool sendBytes(uint8_t data[], uint16_t len, uint16_t timeout = 10);

    static CAN_HandleTypeDef &getHcan() { return PortCAN::hcan_; }
    static uint16_t getModuleID() { return PortCAN::module_id_; }
    static void setModuleID(uint16_t id) { PortCAN::module_id_ = id; }

private:
    static uint16_t module_id_;
    static CAN_HandleTypeDef hcan_;
};

#endif // CENTER_MODULE_PORT_CAN_H_