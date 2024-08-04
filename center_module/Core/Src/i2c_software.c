#include "stm32f4xx_hal.h"
#include "i2c_software.h"
#include "gpio.h"

#define I2C_SCL_PIN GPIO_PIN_10
#define I2C_SDA_PIN GPIO_PIN_11
#define I2C_GPIO_PORT GPIOB

#define I2C_SCL_HIGH() HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_LOW() HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)
#define I2C_SDA_HIGH() HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_LOW() HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)
#define I2C_SDA_READ() HAL_GPIO_ReadPin(I2C_GPIO_PORT, I2C_SDA_PIN)

static void I2C_Delay(void) {
    for (volatile int i = 0; i < 30; i++);
}

void I2C_Software_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStruct);

    I2C_SCL_HIGH();
    I2C_SDA_HIGH();
}

void I2C_Software_Start(void) {
    I2C_SDA_HIGH();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_LOW();
    I2C_Delay();
    I2C_SCL_LOW();
    I2C_Delay();
}

void I2C_Software_Stop(void) {
    I2C_SDA_LOW();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_HIGH();
    I2C_Delay();
}

void I2C_Software_Restart(void) {
    I2C_SDA_HIGH();
    I2C_SCL_LOW();
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_LOW();
    I2C_Delay();
    I2C_SCL_LOW();
    I2C_Delay();
}

void I2C_Software_WriteBit(uint8_t bit) {
    if (bit) {
        I2C_SDA_HIGH();
    } else {
        I2C_SDA_LOW();
    }
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SCL_LOW();
    I2C_Delay();
}

uint8_t I2C_Software_ReadBit(void) {
    uint8_t bit;
    I2C_SDA_HIGH();
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    bit = I2C_SDA_READ();
    I2C_SCL_LOW();
    I2C_Delay();
    return bit;
}

void I2C_Software_WriteByte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        I2C_Software_WriteBit((byte & 0x80) != 0);
        byte <<= 1;
    }
    I2C_Software_ReadBit(); // ACK bit
}

uint8_t I2C_Software_ReadByte(uint8_t ack) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte = (byte << 1) | I2C_Software_ReadBit();
    }
    I2C_Software_WriteBit(ack ? 0 : 1); // ACK bit
    return byte;
}

uint8_t I2C_Software_ReadAck(void) {
    uint8_t ack;
    I2C_SDA_HIGH();
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    ack = I2C_SDA_READ();
    I2C_SCL_LOW();
    I2C_Delay();
    return ack;
}