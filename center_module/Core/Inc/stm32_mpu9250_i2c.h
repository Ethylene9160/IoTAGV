#include <stdint.h>
#include "main.h"

#ifndef _STM32_MPU9250_I2C_H_
#define _STM32_MPU9250_I2C_H_

#define I2C_SCL_Pin GPIO_PIN_10
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_11
#define I2C_SDA_GPIO_Port GPIOB

#ifdef __cplusplus
extern "C" {
#endif

int stm32_i2c_write(unsigned char slave_addr, unsigned char reg_addr,
                       unsigned char length, unsigned char * data);
int stm32_i2c_read(unsigned char slave_addr, unsigned char reg_addr,
                       unsigned char length, unsigned char * data);

#ifdef __cplusplus
}
#endif
#endif // _STM32_MPU9250_I2C_H_