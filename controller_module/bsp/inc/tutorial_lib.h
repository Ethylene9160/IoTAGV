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

#ifndef LIBRM_H_
#define LIBRM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dev.h"


/**
 * @brief 发送控制指令
 * @note 这些函数都被实现在了 `librm_lib_1.0.a` 的 `tutorial_lib.o` 中.
 *       TODO: recv_id 是什么, 下同
 */
void led_ctrl_data(uint8_t recv_id, led_ctrl_t ctrl_data);
void buzzer_ctrl_data(uint8_t recv_id, buzzer_ctrl_t ctrl_data);
void num_ctrl_data(uint8_t recv_id, num_ctrl_t ctrl_data);
void relay_ctrl_data(uint8_t recv_id, relay_ctrl_t ctrl_data);
void servo_ctrl_data(uint8_t recv_id, servo_ctrl_t ctrl_data);
void motor_ctrl_data(uint8_t recv_id, motor_ctrl_t ctrl_data);
void wireless_config_data(uint8_t recv_id, wireless_config_t config_data);
void wireless_send_data(uint8_t recv_id, wireless_data_t send_data);

/**
 * @brief 发送获取消息请求
 * @note 这些函数都被实现在了 `librm_lib_1.0.a` 的 `tutorial_lib.o` 中. 内部都是调用 send_can_pack_cmd.
 */
void motor_mesg_request(uint8_t recv_id);
void poten_mesg_request(uint8_t recv_id);
void sonic_mesg_request(uint8_t recv_id);
void gyro_mesg_request(uint8_t recv_id);
void stick_mesg_request(uint8_t recv_id);
void hall_mesg_request(uint8_t recv_id);

/**
 * @brief 消息回调函数
 * @note 这些函数都在 `librm_lib_1.0.a` 的 `tutorial_lib.o` 中被作为外部符号调用, 可由用户覆写实现.
 *       在 `librm_lib_1.0.a` 的 `protocol_parser.o` 中有 WEAK 的空实现, 故不使用的可以不写.
 *       静态库中将 UART 和 CAN 接口的消息全都统一, 允许互相转发和作为指令执行, 对于后者用户可以自己注册指令对应的回调函数, 消息帧格式与注册机制详见对应注释 (目前在 `threads.c`).
 */
void motor_mesg_handler(uint8_t send_id, motor_sensor_t *data); // `tutorial_lib.o` 里没有该函数, 详见 `motor.c`
void infrared_mesg_handler(uint8_t send_id, infrared_sensor_t *data);
void poten_mesg_handler(uint8_t send_id, poten_sensor_t *data);
void sonic_mesg_handler(uint8_t send_id, sonic_sensor_t *data);
void gyro_mesg_handler(uint8_t send_id, gyro_sensor_t *data);
void stick_mesg_handler(uint8_t send_id, stick_sensor_t *data);
void hall_mesg_handler(uint8_t send_id, hall_sensor_t *data);
void key_mesg_handler(uint8_t send_id, key_sensor_t *data);
void wireless_recv_handler(uint8_t send_id, wireless_data_t *data);

/**
 * @brief dbus 串口消息回调函数
 * @note 该函数可由用户覆写实现, 分析详见 bsp_usart.h 中 `uart_recv_handler` 的注释.
 */
void dbus_uart_handler(uint8_t *buff);

#ifdef __cplusplus
};
#endif

#endif