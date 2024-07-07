#include "msgs.h"

#include "string.h"


uint16_t gen_ranging_exchange_msg(
    uint8_t *buf,
    uint16_t pan_id,
    uint8_t dest_id,
    uint8_t src_id,
    uint8_t task_id,
    uint8_t msg_type,
    uint8_t *payload,
    uint16_t payload_len
) {
    buf[0] = RANGING_EXCHANGE_MSG_SYNC_BYTE;
    buf[1] = pan_id & 0xFF;
    buf[2] = (pan_id >> 8) & 0xFF;
    buf[3] = dest_id;
    buf[4] = src_id;
    buf[5] = task_id;
    buf[6] = msg_type;
    memcpy(buf + 7, payload, payload_len);
    memset(buf + 7 + payload_len, 0, 2);
    return payload_len + 9;
}

uint16_t re_get_pan_id(const uint8_t *buf) {
    return (buf[2] << 8) | buf[1];
}

uint8_t re_get_dest_id(const uint8_t *buf) {
    return buf[3];
}

uint8_t re_get_src_id(const uint8_t *buf) {
    return buf[4];
}

uint8_t re_get_task_id(const uint8_t *buf) {
    return buf[5];
}

uint8_t re_get_msg_type(const uint8_t *buf) {
    return buf[6];
}

uint16_t re_get_payload_head_index() {
    return 7;
}
