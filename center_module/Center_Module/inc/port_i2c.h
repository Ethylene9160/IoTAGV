#ifndef PORT_I2C_H_
#define PORT_I2C_H_

#include <stdint.h>

class PortI2C {
public:
    PortI2C(int slave_addr);
    ~PortI2C();

    void write_byte(uint8_t data);
    int read_byte();
    void write_register_8(uint8_t reg, uint8_t data);
    uint8_t read_register_8(uint8_t reg);
    void write_register_16(uint8_t reg, uint16_t data);
    uint16_t read_register_16(uint8_t reg);

private:
    uint8_t slave_addr;
    // TODO
};

#endif