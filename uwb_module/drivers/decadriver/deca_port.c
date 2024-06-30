/*! ----------------------------------------------------------------------------
 * @file	port.h
 * @brief	HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "deca_port.h"

#include "deca_sleep.h"
#include "spi.h"


void reset_DW1000(void) {
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

void spi_set_rate_low(void) {
    ChangeSPIRate(SPI_BaudRatePrescaler_32);
}

void spi_set_rate_high(void) {
    ChangeSPIRate(SPI_BaudRatePrescaler_4);
}
