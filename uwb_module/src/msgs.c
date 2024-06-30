#include "msgs.h"

#include "string.h"


uint16_t gen_ranging_exchange_msg(uint8_t *buf, uint16_t pan_id, uint16_t dest_id, uint16_t src_id, uint8_t task_id, uint8_t msg_type, uint8_t *payload, uint16_t payload_len) {
    buf[0] = RANGING_EXCHANGE_MSG_SYNC_BYTE;
    buf[1] = pan_id & 0xFF;
    buf[2] = (pan_id >> 8) & 0xFF;
    buf[3] = dest_id & 0xFF;
    buf[4] = (dest_id >> 8) & 0xFF;
    buf[5] = src_id & 0xFF;
    buf[6] = (src_id >> 8) & 0xFF;
    buf[7] = task_id;
    buf[8] = msg_type;
    memcpy(buf + 9, payload, payload_len);
    memset(buf + 9 + payload_len, 0, 2);
    return payload_len + 11;
}
