#ifndef UWB_MODULE_SYSTICK_H
#define UWB_MODULE_SYSTICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"


uint32_t GetSystickMs(void);
void SleepMs(unsigned int time_ms);
int ConfigureSysTick(void);

#ifdef __cplusplus
}
#endif

#endif