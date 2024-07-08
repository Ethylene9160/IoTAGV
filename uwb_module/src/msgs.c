#include "msgs.h"

#include "string.h"

#include "tiny_io.h"


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

void send_upload_position_msg(uint8_t src_id, float x, float y, float d0, float d1) {
    uart_send_byte(0x5A); // Head
    uart_send_byte(src_id); // src_id (L)
    uart_send_byte(0x00); // src_id (H)
    uart_send_byte(0x00); // dest_id (L)
    uart_send_byte(0x00); // dest_id (H)
    uart_send_byte(0x00); // CRC8 (not used)
    uart_send_byte(0x10); // data_len (L)
    uart_send_byte(0x00); // data_len (H)
    uint8_t payload[16] = {0};
    memcpy(payload, &x, 4);
    memcpy(payload + 4, &y, 4);
    memcpy(payload + 8, &d0, 4);
    memcpy(payload + 12, &d1, 4);
    for (int i = 0; i < 16; i ++) {
        uart_send_byte(payload[i]);
    }
    uart_send_byte(0x7F); // Tail
}
