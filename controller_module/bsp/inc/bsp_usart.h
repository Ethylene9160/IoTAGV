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

/**
 * `MX_USART1_UART_Init` 和 `MX_USART3_UART_Init` 都在 `librm_lib_1.0.a` 的 `usart.o` 中.
 * 反编译二者分别可找到 `MOV.W R3, #0xE1000` 和 `LDR R3, =0x186A0`, 此即波特率, UART1 为 921600, UART3 为 100000.
 */

#ifndef BSP_USART_H_
#define BSP_USART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"


/**
 * @brief 注册 UART1 的接收回调函数 (和回调函数类型定义)
 * @param fun 回调函数
 * @note 该函数被实现在了 `librm_lib_1.0.a` 的 `bsp_usart.o` 中.
 *       注意在 `librm_lib_1.0.a` 的 `protocol_parser.o` 中, `init_uart_rx_fifo` 已经将该回调设为了 `uart_data_to_fifo`, 而 `init_uart_rx_fifo` 在 `main.o` 的主函数中被调用了.
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
 *       即该静态库固定了 uart1 用于普通的串口通信 (这里是接收), uart3 作为 DBus.
 *       在调用回调函数后, 还使用 `osSignalSet` 给 `unpack_task` 任务 (见 `threads.c` 注释) 发送信号量 2, 会使其调用 `pc_unpack_handler` (`protocol_parser.o`, 这么看这个 `pc_` 可能指的是上位机),
 *       其中如果第二个字节 (recv_type) 是 0xF0 且第三个字节 (recv_id) 为 0x00, 或者这两个字节都是 0xFF (这种情况会既转发又执行指令), 则该消息为指令, 会调用 `Module_Cmd_Handle`,
 *       此外, 其中如果消息第二个字节 (recv_type) 是 0x80, 则会再调用 `Forward_Packet` (`protocol_parser.o`), 跳到 `bsp_uart1_sendbuf`, 转发给 UART1; 否则, 如果第四个字节 (send_type) 是 0x80 则转发给 CAN.
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
 * @note `librm_lib_1.0.a` 的 `bsp_usart.o` 中并没有实现该函数, 只有一个空架子.
 */
void debug_printf(const char *pstr, ...);

#ifdef __cplusplus
};
#endif

#endif