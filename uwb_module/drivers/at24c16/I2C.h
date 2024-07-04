
#include "stm32f10x.h"

#ifndef I2C_H_
#define I2C_H_

extern unsigned char SlaveAddress;

void I2C_GPIO_Config(void);
void Single_WriteI2C(unsigned int REG_Address,unsigned char REG_data);//单字节写入
unsigned char Single_ReadI2C(unsigned int REG_Address);//读取单字节
void delay5ms(void);

#endif /* AT24C02_H_ */
