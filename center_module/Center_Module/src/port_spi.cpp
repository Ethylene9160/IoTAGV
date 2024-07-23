#include "port_spi.h"

PortSPI::PortSPI(int channel) {
    this->channel = channel;

    // TODO
}

PortSPI::~PortSPI() {
    // TODO
}

void PortSPI::send_byte(uint8_t byte) {
    // TODO
}

uint8_t PortSPI::receive_byte() {
    // TODO
    return 0;
}

void PortSPI::send_word(uint16_t word) {
    // TODO
}

uint16_t PortSPI::receive_word() {
    // TODO
    return 0;
}

void PortSPI::send_data(uint8_t* data, int length) {
    // TODO
}

void PortSPI::receive_data(uint8_t* data, int length) {
    // TODO
}
