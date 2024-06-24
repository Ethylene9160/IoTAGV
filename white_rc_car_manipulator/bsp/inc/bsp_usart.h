/****************************************************************************
 *  Copyright (C) 2018 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of鑱�
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.鑱� See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

/**
  *********************** (C) COPYRIGHT 2018 DJI **********************
  * @update
  * @history
  * Version     Date           Author           Modification
  * V1.0.0      June-15-2018   ric.luo
  *             June-25-2024
  * @verbatim
  *********************** (C) COPYRIGHT 2018 DJI **********************
  */

#ifndef BSP_USART_H_
#define BSP_USART_H_

#include "usart.h"

/**
 * @brief 注册 UART1 的接收回调函数 (和回调函数类型定义)
 * @param fun 回调函数
 * @note 该函数被实现在了 `librm_lib_1.0.a` 的 `bsp_usart.o` 中.
 */
typedef void (*uart_callback_t)(uint8_t *buf, uint32_t len);
void uart1_callback_reg(uart_callback_t fun);

/**
 * @brief 通过 UART1 发送数据
 * @note 该函数被实现在了 `librm_lib_1.0.a` 的 `bsp_usart.o` 中.
 *       内部使用 `HAL_UART_Transmit` 实现.
 */
void bsp_uart1_sendbuf(uint8_t* buf, uint16_t len);

/**
 * @brief 供串口中断处理函数调用
 * @param huart 触发中断的串口外设句柄
 * @note 该函数被实现在了 `librm_lib_1.0.a` 的 `bsp_usart.o` 中, 并被 stm32f4xx_it.c (中断) 中的 `USART1_IRQHandler` 和 `USART3_IRQHandler` 调用.
 *       具体地, huart 地址经 CMP 条件判断, 如果是 huart1 则调用 uart1_callback; 如果是 huart3 则调用 `dbus_uart_handler`.
 *       即该静态库固定了 uart1 用于普通的串口通信 (调试输出), uart3 作为 DBus.
 */
void uart_recv_handler(UART_HandleTypeDef *huart);

/**
 * @brief 配置 UART1 和 UART3, 应该主要是关于 DMA 功能.
 * @note 该函数被实现在了 `librm_lib_1.0.a` 的 `bsp_usart.o` 中, 被 `librm_lib_1.0.a` 的 `freertos.o` 中的 `MX_FREERTOS_Init` 调用, 不需要用户来调用.
 *       使用了同样实现于该 .o 中的 `uart_recv_dma_config`.
 */
void bsp_uart_config(void);

/**
 * @brief 调试输出
 * @note 该函数被实现在了 `librm_lib_1.0.a` 的 `bsp_usart.o` 中.
 *       TODO: 静态库中疑似并没有实现该函数, 只有一个空架子, 有待确认
 */
void debug_printf(const char *pstr, ...);

#endif