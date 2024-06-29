/*! ----------------------------------------------------------------------------
 * @file    port.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "deca_sleep.h"
#include "port.h"

extern __IO uint64_t systick_ms;

uint32_t getSystickMs(void) {
    return systick_ms;
}

int configureSysTick(void) {
    if (SysTick_Config(SystemCoreClock / 1000)) { // CLOCKS_PER_SEC = 1000
        /* Capture error */
        while (1);
    }
    NVIC_SetPriority(SysTick_IRQn, 5);
    return 0;
}

void changeSPIRate(uint16_t scalingfactor) {
    uint16_t reg = 0;
    reg = SPIx->CR1; // Get the SPIx CR1 value
    reg &= 0xFFC7; // Clear the scaling bits
    reg |= scalingfactor; // Set the scaling bits
    SPIx->CR1 = reg; // Write to SPIx CR1
}

void setLowSPIRate(void) {
    changeSPIRate(SPI_BaudRatePrescaler_32);
}

void setHighSPIRate(void) {
    changeSPIRate(SPI_BaudRatePrescaler_4);
}

int configureSPI(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

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
    SPI_InitStructure.SPI_BaudRatePrescaler = SPIx_PRESCALER; // SPIx_PRESCALER = SPI_BaudRatePrescaler_8
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

int configureGPIO(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure all unused GPIO port pins in Analog Input mode (floating input
    * trigger OFF), this will reduce the power consumption and increase the device
    * immunity against EMI/EMC */

    // Enable GPIOs clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);

    // Set all GPIO pins as analog inputs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    return 0;
}

void resetDW1000(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable GPIO used for DW1000 reset
    GPIO_InitStructure.GPIO_Pin = DW1000_RSTn;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);

    // Drive the RSTn pin low
    GPIO_WriteBit(GPIOB, DW1000_RSTn, 0);
    deca_sleep(100);
    GPIO_WriteBit(GPIOB, DW1000_RSTn, 1);
    deca_sleep(100);

    // Put the pin back to tri-state ... as input
    GPIO_InitStructure.GPIO_Pin = DW1000_RSTn;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);
}

void turnOffLED(led_t led) {
    switch (led) {
        case LED_PA0:
            GPIO_ResetBits(GPIOA, GPIO_Pin_1);
            break;
        case LED_PA2:
            GPIO_ResetBits(GPIOA, GPIO_Pin_2);
            break;
        case LED_PA3:
            GPIO_ResetBits(GPIOA, GPIO_Pin_3);
            break;
        case LED_ALL:
            GPIO_ResetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
            break;
        default:
            // do nothing for undefined led number
            break;
    }
}

void turnOnLED(led_t led) {
    switch (led) {
        case LED_PA0:
            GPIO_SetBits(GPIOA, GPIO_Pin_0);
            break;
        case LED_PA2:
            GPIO_SetBits(GPIOA, GPIO_Pin_2);
            break;
        case LED_PA3:
            GPIO_SetBits(GPIOA, GPIO_Pin_3);
            break;
        case LED_ALL:
            GPIO_SetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3);
            break;
        default:
            // do nothing for undefined led number
            break;
    }
}

void configureUSART(void) {
    /* USARTx configured as follow:
          - BaudRate = 115200 baud
          - Word Length = 8 Bits
          - One Stop Bit
          - No parity
          - Hardware flow control disabled (RTS and CTS signals)
          - Receive and transmit enabled
    */

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

int __io_putchar(int ch) {
    USART_ClearFlag(USART1,USART_FLAG_TC);
    USART_SendData(USART1, (uint8_t) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); // Block until the transfer is complete
    return ch;
}

//void USART_puts(uint8_t *s,uint8_t len) {
//    int i;
//    for(i=0; i<len; i++)
//    {
//        putchar(s[i]);
//    }
//}

void testLED(void) {
    turnOnLED(LED_ALL);
    deca_sleep(500);
    turnOffLED(LED_ALL);
    deca_sleep(500);
    turnOnLED(LED_ALL);
    deca_sleep(500);
    turnOffLED(LED_ALL);
}

void initPeripherals(void) {
    /* SysTick */
    configureSysTick();

    /* GPIO */
    configureGPIO();

    /* Use SWD */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    /* SPI1 */
    configureSPI();

    /* USART1 */
    configureUSART();

    testLED();
}
