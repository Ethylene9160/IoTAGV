#ifndef __BSP_USART_H__
#define __BSP_USART_H__

#include "sys.h"
#include "usart.h"

typedef void (*uart_callback_t)(uint8_t *buf, uint32_t len);
void uart1_callback_reg(uart_callback_t fun);

void bsp_uart_config(void);

void bsp_uart1_sendbuf(uint8_t* buf, uint16_t len);

void debug_printf(const char *pstr, ...);

void uart_recv_handler(UART_HandleTypeDef *huart);

#endif


