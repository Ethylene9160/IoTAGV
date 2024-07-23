#ifndef PORT_SPI_H_
#define PORT_SPI_H_

#include <stdint.h>

class PortSPI {
public:
    PortSPI(int channel);
    ~PortSPI();

    void send_byte(uint8_t byte);
    uint8_t receive_byte();
    void send_word(uint16_t word);
    uint16_t receive_word();
    void send_data(uint8_t* data, int length);
    void receive_data(uint8_t* data, int length);

private:
    int channel;
};

#endif