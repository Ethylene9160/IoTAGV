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
    bool active_low = false;
    uint32_t last_pressed_time = 0;
public:
	UserGPIO(GPIO_TypeDef *_port, uint16_t _pin, bool _active_low = false);
	UserGPIO(char *_pinName, bool _active_low = false);

	void setState(bool _state);
	bool getState();
	void set();
	void reset();
	void toggle();

    bool pressed(uint32_t continuous_time_threshold = 20, uint32_t debounce_time_threshold = 20);
};

#ifdef __cplusplus
}
#endif

#endif
