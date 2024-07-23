#include "port_i2c.h"

PortI2C::PortI2C(int slave_addr) {
    this->slave_addr = slave_addr;

    // TODO
}

PortI2C::~PortI2C() {
    // TODO
}

void PortI2C::write_byte(uint8_t data) {
    // TODO
}

int PortI2C::read_byte() {
    // TODO
    return 0;
}

void PortI2C::write_register_8(uint8_t reg, uint8_t data) {
    // TODO
}

uint8_t PortI2C::read_register_8(uint8_t reg) {
    // TODO
    return 0;
}

void PortI2C::write_register_16(uint8_t reg, uint16_t data) {
    // TODO
}

uint16_t PortI2C::read_register_16(uint8_t reg) {
    // TODO
    return 0;
}
