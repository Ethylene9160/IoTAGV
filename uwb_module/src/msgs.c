#include "msgs.h"

#include <stdio.h>

#include "dwt.h"
#include "string.h"

#include "tiny_io.h"
#include "uwb_module_config.h"


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

/**
 * 打印：src id， dest id， msg type
 * @param buffer buffer.
 * @param len
 */
// void print_msg_string(uint8_t* buffer, uint32_t len) {
//     uint8_t src_id = re_get_src_id(buffer);
//     uint8_t dest_id = re_get_dest_id(buffer);
//     uint8_t msg_type = re_get_msg_type(buffer);
//     debug_printf("%d recv: src: %d, dest: %d, msg_type: %d\n", module_config.module_id, src_id, dest_id, msg_type);
// }
//
//
// void print_distance_msg(uint8_t* buffer) {
//     uint8_t src_id = re_get_src_id(buffer);
//     uint8_t dest_id = re_get_dest_id(buffer);
//     uint8_t msg_type = re_get_msg_type(buffer);
//     float d0 = 0;
//     float d1 = 0;
//     memcpy(&d0, &buffer[7], 4);
//     memcpy(&d1, &buffer[11], 4);
//     debug_printf("src_id: %d, dest_id: %d, msg_type: %d, d0: %f, d1: %f\r\n", src_id, dest_id, msg_type, d0, d1);
// }

void send_upload_position_msg(uint8_t src_id, uint8_t dest_id, uint8_t msg_type, float x, float y, uint8_t* ctrl_info) {
    uart_send_byte(0x5A); // Head
    uart_send_byte(src_id); // src_id (L)
    uart_send_byte(0x00); // src_id (H)
    uart_send_byte(dest_id); // dest_id (L)
    uart_send_byte(0x00); // dest_id (H)
    uart_send_byte(msg_type); // CRC8 (not used)
    uart_send_byte(0x10); // data_len (L)
    uart_send_byte(0x00); // data_len (H)
    uint8_t payload[16] = {0};
    memcpy(payload, &x, 4);
    memcpy(payload + 4, &y, 4);
    // memcpy(payload+8, ctrl_info, 8);
    for (int i = 0; i < 8; i ++) {
        uart_send_byte(payload[i]);
    }
    for (int i = 0; i < 8; i ++) {
        uart_send_byte(ctrl_info[i]);
    }
    uart_send_byte(0x7F); // Tail
}
