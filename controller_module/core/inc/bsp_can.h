#ifndef BSP_CAN_H_
#define BSP_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

typedef void (*can_callback_t)(uint8_t *data, uint32_t id, uint32_t len);
void can_callback_reg(can_callback_t ptr);

#ifdef __cplusplus
};
#endif

#endif