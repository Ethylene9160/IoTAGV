#ifndef UWB_MODULE_UWB_MODULE_CONFIG_H
#define UWB_MODULE_UWB_MODULE_CONFIG_H

#include "stdint.h"

/* TODO: 似乎模块最好竖着放 */

typedef struct _uwb_module_config_t {
    uint8_t module_id;
    uint8_t ranging_exchange_debug_output;
    uint32_t ranging_exchange_poll_interval; /* In milliseconds, for Anchor. */
} uwb_module_config_t;


static uwb_module_config_t module_config = {
    .module_id = 0x80, /* 0x00 - 0x7F: Anchor; 0x80 - 0xFF: Tag. */
    .ranging_exchange_debug_output = 0,
    .ranging_exchange_poll_interval = 100
};

#endif