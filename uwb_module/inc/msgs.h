#ifndef UWB_MODULE_MSGS_H
#define UWB_MODULE_MSGS_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RANGING_EXCHANGE_MSG_SYNC_BYTE 0x5A

#define RANGING_EXCHANGE_MSG_TASK_ID_MAX 128

#define RANGING_EXCHANGE_MSG_TYPE_POLL 0x01
#define RANGING_EXCHANGE_MSG_TYPE_RESPONSE 0x02
#define RANGING_EXCHANGE_MSG_TYPE_FINAL 0x03

uint16_t gen_ranging_exchange_msg(uint8_t *buf, uint16_t pan_id, uint16_t dest_id, uint16_t src_id, uint8_t task_id, uint8_t msg_type, uint8_t *payload, uint16_t payload_len);

#ifdef __cplusplus
}
#endif

#endif