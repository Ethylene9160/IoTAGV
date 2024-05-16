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

#ifndef __TUTORIAL_LIB_H__
#define __TUTORIAL_LIB_H__

#include "dev.h"

/**
  * @brief    Wait for Timeout (Time Delay)
  * @param    millisec      time delay value
  */
void task_delay(uint32_t millisec);


/********************** MST request ************************/
void    led_ctrl_data(uint8_t recv_id, led_ctrl_t    ctrl_data);
void buzzer_ctrl_data(uint8_t recv_id, buzzer_ctrl_t ctrl_data);
void    num_ctrl_data(uint8_t recv_id, num_ctrl_t    ctrl_data);
void  relay_ctrl_data(uint8_t recv_id, relay_ctrl_t  ctrl_data);
void  servo_ctrl_data(uint8_t recv_id, servo_ctrl_t  ctrl_data);
void  motor_ctrl_data(uint8_t recv_id, motor_ctrl_t  ctrl_data);

void wireless_config_data(uint8_t recv_id, wireless_config_t config_data);
void   wireless_send_data(uint8_t recv_id, wireless_data_t   send_data);

/********************** REQ request ************************/
void    motor_mesg_request(uint8_t recv_id);
void    poten_mesg_request(uint8_t recv_id);
void    sonic_mesg_request(uint8_t recv_id);
void     gyro_mesg_request(uint8_t recv_id);
void    stick_mesg_request(uint8_t recv_id);
void     hall_mesg_request(uint8_t recv_id);

void    motor_mesg_handler(uint8_t send_id, motor_sensor_t    *data);
void infrared_mesg_handler(uint8_t send_id, infrared_sensor_t *data);
void    poten_mesg_handler(uint8_t send_id, poten_sensor_t    *data);
void    sonic_mesg_handler(uint8_t send_id, sonic_sensor_t    *data);
void     gyro_mesg_handler(uint8_t send_id, gyro_sensor_t     *data);
void    stick_mesg_handler(uint8_t send_id, stick_sensor_t    *data);
void     hall_mesg_handler(uint8_t send_id, hall_sensor_t     *data);

/************************* ACK *****************************/
void      key_mesg_handler(uint8_t send_id, key_sensor_t      *data);
void wireless_recv_handler(uint8_t send_id, wireless_data_t   *data);


/************************* UART *****************************/
void     dbus_uart_handler(uint8_t *buff);
void     dbus_mesg_handler(rc_info_t *rc, uint8_t *buff);



#endif


