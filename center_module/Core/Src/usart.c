/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "queue.h"

#define BUFFER_SIZE_WITH_TIMESTAMP 27
#define REMOTE_RX_MAX_SIZE 13

// DEFINE A LARGE BUFFER.
uint8_t u1_rx_buffer[BUFFER_SIZE<<3];
uint8_t u2_rx_buffer[BUFFER_SIZE<<2];
volatile uint8_t usart1_buffer_index = 0;
volatile uint8_t usart2_buffer_index = 0;
QueueHandle_t S_Queue; // get the message from uwb
QueueHandle_t Remote_Queue; // get the message from remote machine
osMutexId_t USART1_MutexHandle;
osMutexId_t USART2_MutexHandle;
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USART1 init function */

void MX_USART1_UART_Init(void) {
    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    // Enable the UART Data Register not empty Interrupt
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    /* USER CODE BEGIN USART1_Init 2 */
    S_Queue = xQueueCreate(25, BUFFER_SIZE_WITH_TIMESTAMP * sizeof(uint8_t));

    const osMutexAttr_t USART_MutexAttr = {
        .name = "USART_Mutex"
    };
    USART1_MutexHandle = osMutexNew(&USART_MutexAttr);

    Remote_Queue = xQueueCreate(25, REMOTE_RX_MAX_SIZE * sizeof(uint8_t));
    const osMutexAttr_t USART2_MutexAttr = {
        .name = "USART2_Mutex"
    };
    USART2_MutexHandle = osMutexNew(&USART2_MutexAttr);
    /* USER CODE END USART1_Init 2 */
}

/* USART2 init function */

void MX_USART2_UART_Init(void) {
    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */

    /* USER CODE BEGIN USART2_Init 2 */

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
    // Enable the UART Data Register not empty Interrupt
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    /* USER CODE END USART2_Init 2 */
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */
        /* USART1 clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    } else if (uartHandle->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */
        /* USART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {
    if (uartHandle->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        /* USART1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    } else if (uartHandle->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

        /* USART2 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
void USART1_IRQHandler(void) {
    static uint8_t flag = 0;
    static uint8_t last_receive_byte = 0x7F;
    uint8_t received_byte;
    uint8_t buffer[27];
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_UART_IRQHandler(&huart1);

    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE)) {
        HAL_UART_Receive(&huart1, &received_byte, 1, 0);
        if (flag == 0) {
            if (received_byte == 0x5A && last_receive_byte == 0x7F) {
                flag = 1;
                usart1_buffer_index = 0;
            } else {
                last_receive_byte = received_byte;
                return;
            }
        }
        u1_rx_buffer[usart1_buffer_index++] = received_byte;

        if (usart1_buffer_index >= BUFFER_SIZE) {
            usart1_buffer_index = 0;
            if (u1_rx_buffer[0] == 0x5A && u1_rx_buffer[24] == 0x7F) {
                memcpy(buffer, u1_rx_buffer, 24);
                buffer[26] = 0x7F;
                if (xQueueSendFromISR(S_Queue, buffer, &xHigherPriorityTaskWoken) != pdTRUE) {
                    Error_Handler();
                }
            }else {
                flag = 0;
            }
        }
        last_receive_byte = received_byte;
    }
}


void USART233_IRQHandler(void) {
    static uint8_t flag = 0;
    static uint8_t last_receive_byte = 0x7F;
    uint8_t received_byte;
    uint8_t buffer[27];
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_UART_IRQHandler(&huart2);

    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) {
        HAL_UART_Receive(&huart2, &received_byte, 1, 0);
        if (flag == 0) {
            if (received_byte == 0x5A && last_receive_byte == 0x7F) {
                flag = 1;
                usart1_buffer_index = 0;
            } else {
                last_receive_byte = received_byte;
                return;
            }
        }
        u1_rx_buffer[usart1_buffer_index++] = received_byte;

        if (usart1_buffer_index >= BUFFER_SIZE) {
            usart1_buffer_index = 0;
            if (u1_rx_buffer[0] == 0x5A && u1_rx_buffer[24] == 0x7F) {
                memcpy(buffer, u1_rx_buffer, 24);
                buffer[26] = 0x7F;
                if (xQueueSendFromISR(S_Queue, buffer, &xHigherPriorityTaskWoken) != pdTRUE) {
                    Error_Handler();
                }
                // HAL_UART_Transmit(&huart1, u1_rx_buffer, BUFFER_SIZE, HAL_MAX_DELAY);
            }else {
                flag = 0;
            }
        }
        last_receive_byte = received_byte;
    }
}
/**
 * This is used for receving remote msg.
 */
void USART2_IRQHandler(void) {
    static uint8_t flag = 0;
    static uint8_t last_receive_byte = 0x7F;
    /**
     *0: 0x5A
     *1: type
     *2: id
     *3-7: float x
     *8-11: float y
     *12: 0x7F
     *total length = 13.
     */
    uint8_t received_byte;
    uint8_t buffer[REMOTE_RX_MAX_SIZE];

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_UART_IRQHandler(&huart2);
    // static uint8_t s2[] = {'d','b','\r','\n'};
    // HAL_UART_Transmit(&huart2, s2, 4, 0xffffffff);
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) {
        HAL_UART_Receive(&huart2, &received_byte, 1, 0);
        if (flag == 0) {
            if (received_byte == 0x5A && last_receive_byte == 0x7F) {
                flag = 1;
                usart2_buffer_index = 0;
            } else {
                last_receive_byte = received_byte;
                return;
            }
        }
        u2_rx_buffer[usart2_buffer_index++] = received_byte;

        if (usart2_buffer_index >= REMOTE_RX_MAX_SIZE) {
            usart2_buffer_index = 0;
            if (u2_rx_buffer[0] == 0x5A && u2_rx_buffer[REMOTE_RX_MAX_SIZE-1] == 0x7F) {
                memcpy(buffer, u2_rx_buffer, REMOTE_RX_MAX_SIZE);
                if (xQueueSendFromISR(Remote_Queue, buffer, &xHigherPriorityTaskWoken) != pdTRUE) {
                    Error_Handler();
                    // HAL_UART_Transmit(&huart2, "failed!", 7, HAL_MAX_DELAY);
                }else {
                    // HAL_UART_Transmit(&huart2, "succeed", 7, HAL_MAX_DELAY);
                }
                // HAL_UART_Transmit(&huart1, u2_rx_buffer, REMOTE_RX_MAX_SIZE, HAL_MAX_DELAY);
            }else {
                flag = 0;
            }
        }
        last_receive_byte = received_byte;
    }
}

/* USER CODE END 1 */
