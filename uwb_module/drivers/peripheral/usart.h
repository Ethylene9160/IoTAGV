#ifndef UWB_MODULE_USART_H
#define UWB_MODULE_USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"


// USART1 - 4-pin header UART interfaces
#define USARTx				        USART1
#define USARTx_RCC                  RCC_APB2Periph_USART1
#define USARTx_GPIO					GPIOA
#define USARTx_TX					GPIO_Pin_9
#define USARTx_RX					GPIO_Pin_10
#define USARTx_BAUDRATE				115200

void ConfigureUSART(void);

void debug_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif