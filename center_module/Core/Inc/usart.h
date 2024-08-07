/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

  /* Includes ------------------------------------------------------------------*/
#include "main.h"

  /* USER CODE BEGIN Includes */
#include <stdint.h>

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

  /* USER CODE BEGIN Private defines */

extern QueueHandle_t S_Queue;
extern QueueHandle_t Remote_Queue;
extern osMutexId_t USART1_MutexHandle;
extern osMutexId_t USART2_MutexHandle;

#define BUFFER_SIZE 25
// #define BUFFER_SIZE 16
  extern uint8_t u1_rx_buffer[BUFFER_SIZE<<3];
  extern uint8_t u2_rx_buffer[BUFFER_SIZE<<2];
  extern volatile uint8_t usart1_buffer_index;
  extern volatile uint8_t usart2_buffer_index;
  /* USER CODE END Private defines */

  void MX_USART1_UART_Init(void);
  void MX_USART2_UART_Init(void);

  /* USER CODE BEGIN Prototypes */
  void USART1_IRQHandler(void);
  /* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
