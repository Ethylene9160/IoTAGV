#include "user_gpio.h"

#include <stdio.h>
#include <string.h>

UserGPIO::UserGPIO(GPIO_TypeDef *_port, uint16_t _pin) {
	port = _port;
	pin = _pin;
}

UserGPIO::UserGPIO(char *_pinName) {
//	assert(strlen(_pinName) >= 2 && _pinName[0] >= 'A' && _pinName[0] <= 'I');
	char portName;
	int pinIdx;
	sscanf(_pinName, "%c%d", &portName, &pinIdx);
	port = ((GPIO_TypeDef *) (GPIOA_BASE + 0x0400UL * (portName - 'A')));
	pin = GPIO_PIN_0 << pinIdx;
}

void UserGPIO::setState(bool _state) {
	HAL_GPIO_WritePin(port, pin, (GPIO_PinState) _state);
}

bool UserGPIO::getState() {
	return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
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
