#ifndef CENTER_MODULE_HMC5883L_H
#define CENTER_MODULE_HMC5883L_H

#include <cstdio>

#include "../../../../../Users/glver/Desktop/tmp/center_module (未完成的磁力计)/Center_Module/inc/user_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

//class HMC5883L {
//public:
//    HMC5883L(std::shared_ptr<UserI2CMaster> _bus, uint8_t _addr) {
//        bus = _bus;
//        addr = _addr;
//    }
//    ~HMC5883L() = default;
//
//    void init() {
//        bus->start();
//        bus->sendByte(addr << 1);
//        bus->sendByte(0x00);
//        bus->sendByte(0x70);
//        bus->stop();
//    }
//public:
//    std::shared_ptr<UserI2CMaster> bus;
//    uint8_t addr;
//};

#define HMC5883L_OK                 1
#define HMC5883L_ERROR              0

#define bsp_DelayMS                 HAL_Delay
#define HMC5883L_SLAVE_ADDRESS      0x3C

#define DATA_OUT_X                  0x03

typedef struct {
    int16_t X;
    int16_t Y;
    int16_t Z;

    int16_t X_Min;
    int16_t Y_Min;
    int16_t Z_Min;

    int16_t X_Max;
    int16_t Y_Max;
    int16_t Z_Max;

    uint8_t Status;

    uint8_t CfgRegA;
    uint8_t CfgRegB;
    uint8_t CfgRegC;
    uint8_t ModeReg;

    uint8_t IDReg[3 + 1];
} HMC5883L_T;

extern HMC5883L_T g_tMag;

void hmc5883l_Init();
void hmc5883l_ReadData();
float hmc5883l_GetAngle();

#ifdef __cplusplus
}
#endif

#endif