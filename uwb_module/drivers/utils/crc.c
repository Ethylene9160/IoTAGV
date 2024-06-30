#include "crc.h"


uint8_t GetCRC8Checksum(const uint8_t *data, uint32_t len, uint8_t init) {
    uint8_t value = init;
    for (uint32_t i = 0; i < len; i ++) {
        value = kCRC8Table[value ^ data[i]];
    }
    return value;
}

uint16_t GetCRC16Checksum(const uint8_t *data, uint32_t len, uint16_t init) {
    uint16_t value = init;
    for (uint32_t i = 0; i < len; i ++) {
        uint8_t byte = data[i];
        value = (kCRC16Table[(byte ^ (value & 0xFF))] ^ (value >> 8)) & 0xFFFF;
    }
    return value;
}