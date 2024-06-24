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
  * @verbatim
  *********************** (C) COPYRIGHT 2018 DJI **********************
  */

#ifndef RM_TUTORIAL_LIB_H_
#define RM_TUTORIAL_LIB_H_

#include "dev.h"


/**
 * @brief 令线程等待一段时间
 * @param millisec
 * @note 实现在了 `librm_lib_1.0.a` 的 `freertos.o` 中, 实际上就是调用了一下 osDelay. (TODO: 不用它的)
 */
void task_delay(uint32_t millisec);


/********************** MST request ************************/
void led_ctrl_data(uint8_t recv_id, led_ctrl_t ctrl_data);
void buzzer_ctrl_data(uint8_t recv_id, buzzer_ctrl_t ctrl_data);
void num_ctrl_data(uint8_t recv_id, num_ctrl_t ctrl_data);
void relay_ctrl_data(uint8_t recv_id, relay_ctrl_t ctrl_data);
void servo_ctrl_data(uint8_t recv_id, servo_ctrl_t ctrl_data);
void motor_ctrl_data(uint8_t recv_id, motor_ctrl_t ctrl_data);
void wireless_config_data(uint8_t recv_id, wireless_config_t config_data);
void wireless_send_data(uint8_t recv_id, wireless_data_t send_data);
// 以上都实现在了 `tutorial_lib.o` 中 (TODO: 有用的标记一下)


/********************** REQ request ************************/
void motor_mesg_request(uint8_t recv_id);
void poten_mesg_request(uint8_t recv_id);
void sonic_mesg_request(uint8_t recv_id);
void gyro_mesg_request(uint8_t recv_id);
void stick_mesg_request(uint8_t recv_id);
void hall_mesg_request(uint8_t recv_id);
// 以上都实现在了 `tutorial_lib.o` 中, 内部都是使用 send_can_pack_cmd 发送请求 (TODO: 有用的标记一下)

void motor_mesg_handler(uint8_t send_id, motor_sensor_t *data); // 这个好像没用, `tutorial_lib.o` 里没有调用 (TODO: 标记一下)
void infrared_mesg_handler(uint8_t send_id, infrared_sensor_t *data);
void poten_mesg_handler(uint8_t send_id, poten_sensor_t *data);
void sonic_mesg_handler(uint8_t send_id, sonic_sensor_t *data);
void gyro_mesg_handler(uint8_t send_id, gyro_sensor_t *data);
void stick_mesg_handler(uint8_t send_id, stick_sensor_t *data);
void hall_mesg_handler(uint8_t send_id, hall_sensor_t *data);


/************************* ACK *****************************/
void key_mesg_handler(uint8_t send_id, key_sensor_t *data);
void wireless_recv_handler(uint8_t send_id, wireless_data_t *data);
// 以上 (直到上一个 `以上`) 都在 `tutorial_lib.o` 中被调用, 需要的话在自己这边覆写之 (TODO: 有用的标记一下)


/************************* UART *****************************/
void dbus_uart_handler(uint8_t *buff);
// 以上在 `bsp_usart.o` 中被 uart_recv_handler 调用 (需要实现), uart_recv_handler 则是在 stm32f4xx_it.c 中断中被调用 (TODO: 留着, 实现放在 dbus.c 中, 将 dbus 作为单独的板上外设与 RC 解耦)
void dbus_mesg_handler(rc_info_t *rc, uint8_t *buff); // 这个和静态库没有联系 (TODO: RC 放到底盘任务或者其它用户代码里去)


#endif