#include "spi.h"


void ChangeSPIRate(uint16_t scaling_factor) {
    uint16_t reg = 0;
    reg = SPIx->CR1; // Get the SPIx CR1 value
    reg &= 0xFFC7; // Clear the scaling bits
    reg |= scaling_factor; // Set the scaling bits
    SPIx->CR1 = reg; // Write to SPIx CR1
}

int ConfigureSPI(void) {
    RCC_APB2PeriphClockCmd(SPIx_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // SPIx SCK and MOSI pin setup
    GPIO_InitStructure.GPIO_Pin = SPIx_SCK | SPIx_MOSI;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPIx_GPIO, &GPIO_InitStructure);

    // SPIx MISO pin setup
    GPIO_InitStructure.GPIO_Pin = SPIx_MISO;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(SPIx_GPIO, &GPIO_InitStructure);

    // SPIx CS pin setup
    GPIO_InitStructure.GPIO_Pin = SPIx_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPIx_CS_GPIO, &GPIO_InitStructure);

    SPI_I2S_DeInit(SPIx);

    // SPIx Mode setup
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPIx, &SPI_InitStructure);

    // Disable SPIx SS Output
    SPI_SSOutputCmd(SPIx, DISABLE);

    // Enable SPIx
    SPI_Cmd(SPIx, ENABLE);

    // Set CS high
    GPIO_SetBits(SPIx_CS_GPIO, SPIx_CS);

    return 0;
}
