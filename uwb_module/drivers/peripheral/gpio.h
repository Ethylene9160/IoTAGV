#ifndef UWB_MODULE_GPIO_H
#define UWB_MODULE_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"

#define LED_GPIO        GPIOA
#define LED_1_PIN       GPIO_Pin_0
#define LED_2_PIN       GPIO_Pin_2
#define LED_3_PIN       GPIO_Pin_3

typedef enum {
    LED_1,
    LED_2,
    LED_3,
    LED_ALL
} led_t;

int ConfigureGPIO(void);
void TurnOffLED(led_t led);
void TurnOnLED(led_t led);
void TestLED(void);

#ifdef __cplusplus
}
#endif

#endif