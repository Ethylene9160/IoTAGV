#ifndef UWB_MODULE_UWB_MODULE_CONFIG_H
#define UWB_MODULE_UWB_MODULE_CONFIG_H

#include "stdint.h"

/* TODO: 似乎模块最好竖着放 */

typedef struct _uwb_module_config_t {
    uint16_t module_id;
    uint8_t ranging_exchange_debug_output;
    uint32_t ranging_exchange_poll_interval; /* In milliseconds, for Anchor. */
} uwb_module_config_t;


static uwb_module_config_t module_config = {
    .module_id = 0x1000,                        /* To be loaded from ROM. 0x0000 - 0x0FFF: Anchor; 0x1000 - 0x1FFF: Tag. */
    .ranging_exchange_debug_output = 0,
    .ranging_exchange_poll_interval = 200
};

#endif