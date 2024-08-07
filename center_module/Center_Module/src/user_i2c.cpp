#include "user_i2c.h"

UserI2CMaster::UserI2CMaster(std::shared_ptr<UserGPIO> _scl, std::shared_ptr<UserGPIO> _sda) {
	scl = _scl;
	sda = _sda;
}

UserI2CMaster::UserI2CMaster(char *_sclName, char *_sdaName) {
	scl = std::make_shared<UserGPIO>(_sclName);
	sda = std::make_shared<UserGPIO>(_sdaName);
}

std::shared_ptr<UserGPIO> UserI2CMaster::getScl() {
	return scl;
}

std::shared_ptr<UserGPIO> UserI2CMaster::getSda() {
	return sda;
}

void i2cDelay() {
	for (int i = 0; i < 0x8f; i ++);
}

void UserI2CMaster::start() {
	sda->set();
	scl->set();
	i2cDelay();
	sda->reset();
	i2cDelay();
	scl->reset();
}

void UserI2CMaster::stop() {
	sda->reset();
	scl->set();
	i2cDelay();
	sda->set();
	i2cDelay();
}

void UserI2CMaster::sendAck(bool _doAck) {
	scl->reset();
	sda->setState(!_doAck);
	i2cDelay();
	scl->set();
	i2cDelay();
	scl->reset();
}

void UserI2CMaster::sendNAck() {
	sendAck(false);
}

bool UserI2CMaster::waitAck(uint32_t _trialOut) {
	sda->set();
	i2cDelay();
	scl->set();
	i2cDelay();
	uint32_t trials = 0;
	while (_trialOut != 0 && sda->getState()) {
		trials += 1;
		if (trials >= _trialOut) {
			stop();
			return false;
		}
	}
	scl->reset();
	return true;
}

bool UserI2CMaster::sendByte(uint8_t _data, uint32_t _trialOut) {
	scl->reset();
	for (int i = 0x80; i != 0; i >>= 1) {
		i2cDelay();
		sda->setState(_data & i);
		i2cDelay();
		scl->set();
		i2cDelay();
		scl->reset();
	}
	return waitAck(_trialOut);
}

uint8_t UserI2CMaster::receiveByte(bool _doAck) {
	uint8_t res = 0;
	sda->set();
	scl->reset();
	for (int i = 0; i < 8; i ++) {
		i2cDelay();
		scl->set();
		res = (res << 1) + sda->getState();
		i2cDelay();
		scl->reset();
	}
	sendAck(_doAck);
	return res;
}

bool UserI2CMaster::sendBytes(uint8_t _devAddr, uint8_t _regAddr, uint16_t _len, uint8_t *_buffer, uint32_t _trialOut) {
	start();
	if (!sendByte((_devAddr << 1) | 0, _trialOut)) return false;
	if (!sendByte(_regAddr, _trialOut)) return false;
	for (int i = 0; i < _len; i ++) {
		if (!sendByte(_buffer[i], _trialOut)) return false;
	}
	stop();
	return true;
}

bool UserI2CMaster::receiveBytes(uint8_t _devAddr, uint8_t _regAddr, uint16_t _len, uint8_t *_buffer, uint32_t _trialOut) {
	return false;
}

bool UserI2CMaster::setRegister(uint8_t _devAddr, uint8_t _regAddr, uint8_t _data, uint32_t _trialOut) {
	start();
	if (!sendByte((_devAddr << 1) | 0, _trialOut)) return false;
	if (!sendByte(_regAddr, _trialOut)) return false;
	if (!sendByte(_data, _trialOut)) return false;
	stop();
	return true;
}

uint8_t UserI2CMaster::getRegister(uint8_t _devAddr, uint8_t _regAddr, uint32_t _trialOut) {
	start();
	if (!sendByte((_devAddr << 1) | 0, _trialOut)) return false;
	if (!sendByte(_regAddr, _trialOut)) return false;
	start();
	if (!sendByte((_devAddr << 1) | 1, _trialOut)) return false;
	uint8_t res = receiveByte(false);
	stop();
	return res;
}
