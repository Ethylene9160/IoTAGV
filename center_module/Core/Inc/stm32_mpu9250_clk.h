#ifndef _STM32_MPU9250_CLK_H_
#define _STM32_MPU9250_CLK_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

int stm32_get_clock_ms(unsigned long *count);
int stm32_delay_ms(unsigned long num_ms);

#ifdef __cplusplus
}
#endif
#endif // _STM32_MPU9250_CLK_H_