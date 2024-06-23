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

#ifndef __RM_DEVICE_H__
#define __RM_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "stdlib.h"
#include "stdint.h"


// 1 led module
typedef struct {
    uint8_t led1: 1;
    uint8_t led2: 1;
    uint8_t led3: 1;
    uint8_t led4: 1;
    uint8_t led5: 1;
    uint8_t led6: 1;
    uint8_t led7: 1;
    uint8_t led8: 1;
} led_ctrl_t;

// 2 key module
typedef struct {
    uint8_t key1: 1;
    uint8_t key2: 1;
    uint8_t key3: 1;
    uint8_t key4: 1;
    uint8_t key5: 1;
    uint8_t key6: 1;
    uint8_t key7: 1;
    uint8_t key8: 1;
    uint8_t key9: 1;
} key_sensor_t;

// 3 buzzer module
typedef struct {
    uint8_t  enable;
    uint16_t freq;      //nHz
} buzzer_ctrl_t;

// 4 digital module
typedef struct {
    uint8_t enable_1;  // 1: en, 0: dis
    uint8_t num_1;     // 0 ~ 9
    uint8_t pnt_1;     // 0 / 1
    uint8_t enable_2;
    uint8_t num_2;
    uint8_t pnt_2;
    uint8_t enable_3;
    uint8_t num_3;
    uint8_t pnt_3;
    uint8_t enable_4;
    uint8_t num_4;
    uint8_t pnt_4;
} num_ctrl_t;

// 5 relay module
typedef struct {
    uint8_t ctrl; // 0: close, 1: open
} relay_ctrl_t;

// 6 servo module
typedef struct {
    uint16_t s1;  // 500 ~ 2500
    uint16_t s2;
    uint16_t s3;
    uint16_t s4;
} servo_ctrl_t;

// 7 motor translate module
typedef struct {
    int8_t  m1_enable;  // 0: dis, 1: en
    int16_t m1_rpm;
    int8_t  m2_enable;
    int16_t m2_rpm;
    int8_t  m3_enable;
    int16_t m3_rpm;
    int8_t  m4_enable;
    int16_t m4_rpm;
} motor_ctrl_t;

typedef struct {
    int32_t m1_cnt;
    int16_t m1_rpm;
    int32_t m2_cnt;
    int16_t m2_rpm;
    int32_t m3_cnt;
    int16_t m3_rpm;
    int32_t m4_cnt;
    int16_t m4_rpm;
} motor_sensor_t;

// 8 infrared module
typedef struct {
    uint8_t in1;    //0: reflex 1: no reflex
    uint8_t in2;
    uint8_t in3;
    uint8_t in4;
} infrared_sensor_t;

// 9 resistance module
typedef struct {
    uint16_t pos;  // 0 ~ 4095
} poten_sensor_t;

// 10 sonic module
typedef struct {
    int16_t  distance;  // mm
} sonic_sensor_t;

// 11 gyro module
typedef struct {
    float gyro_x;  // degree/s
    float gyro_y;  // degree/s
    float gyro_z;  // degree/s
    float angle_x; // degree
    float angle_y; // degree
    float angle_z; // degree
} gyro_sensor_t;

// 12 stick module
typedef struct {
    int16_t pos_x;  //-1000 ~ 1000
    int16_t pos_y;
} stick_sensor_t;

// 13 wireless module
typedef struct {
    uint8_t channel;   // 0 ~ 50
    uint8_t dir;       // 1: rx, 2: tx
} wireless_config_t;

typedef struct {
    uint8_t data[16];
} wireless_data_t;

// 14 hall module
typedef struct {
    uint16_t value;  // 0 ~ 4095
} hall_sensor_t;

// 15 remote control
typedef struct {
    int16_t ch1;
    int16_t ch2;
    int16_t ch3;
    int16_t ch4;
    uint8_t sw1;
    uint8_t sw2;

    struct {
        int16_t x;
        int16_t y;
        int16_t z;
        uint8_t l;
        uint8_t r;
    } mouse;

    uint16_t key_code;
} rc_info_t;

#ifdef __cplusplus
}
#endif

#endif