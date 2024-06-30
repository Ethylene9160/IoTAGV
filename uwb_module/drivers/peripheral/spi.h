#ifndef UWB_MODULE_SPI_H
#define UWB_MODULE_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"


// SPIx - DW1000 SPI interface
#define SPIx				        SPI1
#define SPIx_RCC                    RCC_APB2Periph_SPI1
#define SPIx_GPIO					GPIOA
#define SPIx_CS						GPIO_Pin_4
#define SPIx_CS_GPIO				GPIOA
#define SPIx_SCK					GPIO_Pin_5
#define SPIx_MISO					GPIO_Pin_6
#define SPIx_MOSI					GPIO_Pin_7

void ChangeSPIRate(uint16_t scaling_factor);
int ConfigureSPI(void);

#ifdef __cplusplus
}
#endif

#endif