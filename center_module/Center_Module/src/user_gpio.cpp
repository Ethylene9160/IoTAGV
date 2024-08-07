#include "user_gpio.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"

UserGPIO::UserGPIO(GPIO_TypeDef *_port, uint16_t _pin, bool _active_low) {
	port = _port;
	pin = _pin;
    active_low = _active_low;
}

UserGPIO::UserGPIO(char *_pinName, bool _active_low) {
//	assert(strlen(_pinName) >= 2 && _pinName[0] >= 'A' && _pinName[0] <= 'I');
	char portName;
	int pinIdx;
	sscanf(_pinName, "%c%d", &portName, &pinIdx);
	port = ((GPIO_TypeDef *) (GPIOA_BASE + 0x0400UL * (portName - 'A')));
	pin = GPIO_PIN_0 << pinIdx;
    active_low = _active_low;
}

void UserGPIO::setState(bool _state) {
	HAL_GPIO_WritePin(port, pin, (GPIO_PinState) _state);
}

bool UserGPIO::getState() {
	return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ^ active_low;
}

void UserGPIO::set() {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void UserGPIO::reset() {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void UserGPIO::toggle() {
	HAL_GPIO_TogglePin(port, pin);
}

bool UserGPIO::pressed(uint32_t continuous_time_threshold, uint32_t debounce_time_threshold) {
    if (this->getState()) {
        osDelay(debounce_time_threshold);
        if (this->getState()) {
            // 非抖动误触, 接下来避免连续触发
            uint32_t currentTime = HAL_GetTick();
            if (currentTime - this->last_pressed_time >= continuous_time_threshold) {
                this->last_pressed_time = currentTime;
                return true;
            }
        }
    }
    return false;
}
