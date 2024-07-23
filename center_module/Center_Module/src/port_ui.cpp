#include "port_ui.h"

#include "gpio.h"

#include "port_i2c.h"

/*
 * Notes:
 *  - KEY_CONFIRM_PIN       PA0         按下连通 3V3, 应当下拉;
 *  - KEY_0_PIN             PC1         按下连通 GND, 应当上拉;
 *  - KEY_1_PIN             PC13        按下连通 GND, 应当上拉.
 */

void PortKeys::init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // KEY_CONFIRM
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // KEY_0 & KEY_1
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void PortOLED::init() {

}
