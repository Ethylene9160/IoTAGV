#include "gpio.h"

#include "systick.h"


int ConfigureGPIO(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure all unused GPIO port pins in Analog Input mode (floating input
    * trigger OFF), this will reduce the power consumption and increase the device
    * immunity against EMI/EMC */

    // Enable GPIOs clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);

    // Set all GPIO pins as analog inputs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    return 0;
}

void turnOffLED(led_t led) {
    switch (led) {
        case LED_PA0:
            GPIO_ResetBits(GPIOA, GPIO_Pin_0);
            break;
        case LED_PA2:
            GPIO_ResetBits(GPIOA, GPIO_Pin_2);
            break;
        case LED_PA3:
            GPIO_ResetBits(GPIOA, GPIO_Pin_3);
            break;
        case LED_ALL:
            GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3);
            break;
        default:
            // do nothing for undefined led number
            break;
    }
}

void TurnOnLED(led_t led) {
    switch (led) {
        case LED_PA0:
            GPIO_SetBits(GPIOA, GPIO_Pin_0);
            break;
        case LED_PA2:
            GPIO_SetBits(GPIOA, GPIO_Pin_2);
            break;
        case LED_PA3:
            GPIO_SetBits(GPIOA, GPIO_Pin_3);
            break;
        case LED_ALL:
            GPIO_SetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3);
            break;
        default:
            // do nothing for undefined led number
            break;
    }
}

void TestLED(void) {
    TurnOnLED(LED_ALL);
    SleepMs(100);
    turnOffLED(LED_ALL);
    SleepMs(100);
    TurnOnLED(LED_ALL);
    SleepMs(100);
    turnOffLED(LED_ALL);
}
