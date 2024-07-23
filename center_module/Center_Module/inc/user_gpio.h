#ifndef __USER_GPIO_H
#define __USER_GPIO_H

#include "user_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

class UserGPIO {
private:
	GPIO_TypeDef *port;
	uint16_t pin;
public:
	UserGPIO(GPIO_TypeDef *_port, uint16_t _pin);
	UserGPIO(char *_pinName);

	void setState(bool _state);
	bool getState();
	void set();
	void reset();
	void toggle();
};

#ifdef __cplusplus
}
#endif

#endif
