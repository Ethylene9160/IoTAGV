#include "bytes.h"


void swap_endianness(uint8_t *data, uint32_t length) {
    uint32_t i;
    for (i = 0; i < length / 2; i++) {
        uint8_t temp = data[i];
        data[i] = data[length - 1 - i];
        data[length - 1 - i] = temp;
    }
}
