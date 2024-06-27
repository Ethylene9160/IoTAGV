#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

void send_pc_pack_cmd(uint8_t send_type, uint8_t send_id, uint16_t cmd_id, uint8_t *p_buf, uint16_t len);

void send_can_pack_cmd(uint8_t receiveType, uint8_t receiveID, uint8_t sendType, uint8_t sendID, uint16_t cmd_id, uint8_t *p_buf, uint16_t len);

typedef void (*PTR_FUNC_HANDLER)(uint8_t *p_buf, uint16_t len);
typedef void (*CAN_CMD_FUN_PTR)(uint8_t *p_buf, uint16_t len);
typedef uint8_t CMD_REGISTER_RESULT;
CMD_REGISTER_RESULT module_ack_register(uint16_t cmd_id, CAN_CMD_FUN_PTR fun_ptr);

#ifdef __cplusplus
};
#endif

#endif