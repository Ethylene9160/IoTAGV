#ifndef UWB_MODULE_MSGS_H
#define UWB_MODULE_MSGS_H

#include <stdlib.h>
#include <string.h>
#include <machine/endian.h>

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RANGING_EXCHANGE_MSG_SYNC_BYTE 0x5A

uint16_t gen_ranging_exchange_msg(uint8_t *buf, uint16_t pan_id, uint8_t dest_id, uint8_t src_id, uint8_t task_id, uint8_t msg_type, uint8_t *payload, uint16_t payload_len);
uint16_t re_get_pan_id(const uint8_t *buf);
uint8_t re_get_dest_id(const uint8_t *buf);
uint8_t re_get_src_id(const uint8_t *buf);
uint8_t re_get_task_id(const uint8_t *buf);
uint8_t re_get_msg_type(const uint8_t *buf);
uint16_t re_get_payload_head_index();


void send_upload_position_msg(uint8_t src_id, uint8_t dest_id, uint8_t msg_type, float x, float y, float d0, float d1); // TODO

// void print_msg_string(uint8_t* buffer, uint32_t len); // for debug
//
// void print_distance_msg(uint8_t* buffer); // for debug


//void set_buffer(uint16_t self_id, uint16_t target_id, uint8_t CRC8, uint16_t data_len, uint8_t* data, uint8_t* buffer, uint32_t buffer_len) {
//    buffer[0] = 0x0A;
//    buffer[1] = (self_id >> 8) & 0xFF;
//    buffer[2] = self_id & 0xFF;
//
//    buffer[3] = (target_id >> 8) & 0xFF;
//    buffer[4] = target_id & 0xFF;
//
//    buffer[5] = CRC8;
//    buffer[6] = (data_len >> 8) & 0xFF;
//    buffer[7] = data_len & 0xFF;
//
//    memcpy(&buffer[8], data, data_len);
//
//    // CRC循环荣誉校验
//    uint8_t todo = 0;
//    buffer[8 + data_len] = todo;
//    buffer[9+data_len] = todo;
//
//    buffer[10+data_len] = 0x7F;
//}
//
//void tag_pos_receive(uint8_t* buffer) {
//    //提取x，y：
//    //x: 8-11位，float
//    //y: 12-15位，float
//    // float x = 0;
//    // float y = 0;
//    // memcpy(&x, &buffer[8], 4);
//    // memcpy(&y, &buffer[12], 4);
//
//    uint8_t to_405_buffer[19];
//    memcpy(to_405_buffer, buffer, 16*sizeof(uint8_t));
//
//    // todo: write crc 16
//    uint16_t CRC16 = 0;
//    memcpy(&to_405_buffer[16], &CRC16, sizeof(uint16_t));
//
//    to_405_buffer[19]=0x7F;
//    //todo: 使用uart1发送新的buffer
//
//    //HAL_UART_Transmit(&huart1, (uint8_t*)usart2_buffer, len, HAL_MAX_DELAY);
//}

#ifdef __cplusplus
}
#endif

#endif