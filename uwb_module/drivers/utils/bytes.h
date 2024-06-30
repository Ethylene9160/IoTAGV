#ifndef UWB_MODULE_BYTES_H
#define UWB_MODULE_BYTES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void swap_endianness(uint8_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif