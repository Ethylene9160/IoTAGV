#ifndef __USER_I2C_H
#define __USER_I2C_H

// Configuration: GPIO x 2, Output Open Drain, Pull-up

#include "user_utils.h"

#include "user_gpio.h"
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_ACK_TRIAL_OUT 200

class UserI2CMaster {
private:
	std::shared_ptr<UserGPIO> scl, sda;
public:
	UserI2CMaster(std::shared_ptr<UserGPIO> _scl, std::shared_ptr<UserGPIO> _sda);
	UserI2CMaster(char *_sclName, char *_sdaName);

	std::shared_ptr<UserGPIO> getScl();
	std::shared_ptr<UserGPIO> getSda();

	void start();
	void stop();

	void sendAck(bool _doAck = true);
	void sendNAck();
	bool waitAck(uint32_t _trialOut = I2C_ACK_TRIAL_OUT);

	bool sendByte(uint8_t _data, uint32_t _trialOut = I2C_ACK_TRIAL_OUT);
	uint8_t receiveByte(bool _doAck);

	bool sendBytes(uint8_t _devAddr, uint8_t _regAddr, uint16_t _len, uint8_t *_buffer, uint32_t _trialOut = I2C_ACK_TRIAL_OUT);
	bool receiveBytes(uint8_t _devAddr, uint8_t _regAddr, uint16_t _len, uint8_t *_buffer, uint32_t _trialOut = I2C_ACK_TRIAL_OUT);

	bool setRegister(uint8_t _devAddr, uint8_t _regAddr, uint8_t _data, uint32_t _trialOut = I2C_ACK_TRIAL_OUT);
	uint8_t getRegister(uint8_t _devAddr, uint8_t _regAddr, uint32_t _trialOut = I2C_ACK_TRIAL_OUT);
};

#ifdef __cplusplus
}
#endif

#endif
