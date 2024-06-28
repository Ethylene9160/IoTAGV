#ifndef CRC_H_
#define CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"


#define CRC8_INIT 0x77
#define CRC16_INIT 0x1862


uint8_t Get_CRC8_Check_Sum(uint8_t *pchMessage, uint32_t dwLength, uint8_t ucCRC8);

uint32_t Verify_CRC8_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);

void Append_CRC8_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);

uint16_t Get_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength, uint16_t ucCRC8);

uint32_t Verify_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);

void Append_CRC16_Check_Sum(uint8_t * pchMessage, uint32_t dwLength);

#ifdef __cplusplus
}
#endif

#endif