#ifndef UWB_MODULE_UWB_MODULE_CONFIG_H
#define UWB_MODULE_UWB_MODULE_CONFIG_H

#include "stdint.h"

/* TODO: 似乎模块最好竖着放 */

typedef struct _uwb_module_config_t {
    uint8_t module_id;
    uint8_t ranging_exchange_debug_output;
    uint32_t ranging_exchange_poll_interval; /* In milliseconds, for Anchor. */
    float anchor_x, anchor_y; /* In meters, for Anchor. */
    uint64_t distance_expired_time; /* In milliseconds, for Tag. */
} uwb_module_config_t;


//static uwb_module_config_t module_config = {
//    .module_id = 0x00, /* 0x00 - 0x7F: Anchor; 0x80 - 0xFF: Tag. */
//    .ranging_exchange_debug_output = 0,
//    .ranging_exchange_poll_interval = 100,
//    .anchor_x = 0.0f,
//    .anchor_y = 0.0f
//};

//static uwb_module_config_t module_config = {
//    .module_id = 0x01, /* 0x00 - 0x7F: Anchor; 0x80 - 0xFF: Tag. */
//    .ranging_exchange_debug_output = 0,
//    .ranging_exchange_poll_interval = 100,
//    .anchor_x = 1.5f,
//    .anchor_y = 0.0f
//};

static uwb_module_config_t module_config = {
    .module_id = 0x80, /* 0x00 - 0x7F: Anchor; 0x80 - 0xFF: Tag. */
    .ranging_exchange_debug_output = 0,
//    .ranging_exchange_poll_interval = 0, // TODO: Tag 这里一设置超过 4ms (也就是不让其高速闪灯) 就会算不出距离，不知道为什么
    .distance_expired_time = 1000
};

#endif