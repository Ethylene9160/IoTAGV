#include "gpio.h"

#include "systick.h"


int ConfigureGPIO(void) {
    /* Configure all unused GPIO port pins in Analog Input mode (floating input
    * trigger OFF), this will reduce the power consumption and increase the device
    * immunity against EMI/EMC */

    // Enable GPIOs clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);

    // Set all GPIO pins as analog inputs and configure the LEDs as outputs
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LED_1_PIN | LED_2_PIN | LED_3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_GPIO, &GPIO_InitStructure);

    return 0;
}

void TurnOffLED(led_t led) {
    switch (led) {
        case LED_1:
            GPIO_ResetBits(LED_GPIO, LED_1_PIN);
            break;
        case LED_2:
            GPIO_ResetBits(LED_GPIO, LED_2_PIN);
            break;
        case LED_3:
            GPIO_ResetBits(LED_GPIO, LED_3_PIN);
            break;
        case LED_ALL:
            GPIO_ResetBits(LED_GPIO, LED_1_PIN | LED_2_PIN | LED_3_PIN);
            break;
        default:
            // do nothing for undefined led number
            break;
    }
}

void TurnOnLED(led_t led) {
    switch (led) {
        case LED_1:
            GPIO_SetBits(LED_GPIO, LED_1_PIN);
            break;
        case LED_2:
            GPIO_SetBits(LED_GPIO, LED_2_PIN);
            break;
        case LED_3:
            GPIO_SetBits(LED_GPIO, LED_3_PIN);
            break;
        case LED_ALL:
            GPIO_SetBits(LED_GPIO, LED_1_PIN | LED_2_PIN | LED_3_PIN);
            break;
        default:
            // do nothing for undefined led number
            break;
    }
}

void TestLED(void) {
    TurnOnLED(LED_ALL);
    SleepMs(500);
    TurnOffLED(LED_ALL);
    SleepMs(500);
    TurnOnLED(LED_ALL);
    SleepMs(500);
    TurnOffLED(LED_ALL);
}
