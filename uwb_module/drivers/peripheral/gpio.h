#ifndef UWB_MODULE_GPIO_H
#define UWB_MODULE_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"


typedef enum {
    LED_PA0,
    LED_PA2,
    LED_PA3,
    LED_ALL,
    LEDn
} led_t;

int ConfigureGPIO(void);
void TestLED(void);

#ifdef __cplusplus
}
#endif

#endif