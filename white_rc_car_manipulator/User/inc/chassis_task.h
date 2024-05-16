/****************************************************************************
 *  Copyright (C) 2018 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

/**
  *********************** (C) COPYRIGHT 2018 DJI **********************
  * @update
  * @history
  * Version     Date              Author           Modification
  * V1.0.0      January-15-2018   ric.luo
  * @verbatim
  *********************** (C) COPYRIGHT 2018 DJI **********************
  */ 
 


  
#ifndef __CHASSIS_TASK_H__
#define __CHASSIS_TASK_H__

#include "sys.h"


/**
  * @brief     底盘控制模式枚举
  */
typedef enum
{
  CHASSIS_RELAX,         //底盘失能
  CHASSIS_STOP,          //底盘停止
  CHASSIS_NORMAL,        //底盘正常
  CHASSIS_AVOID,         //底盘避障
} chassis_mode_e;


/**
  * @brief     底盘控制数据结构体
  */
typedef struct
{
  /* 底盘控制模式相关 */
  chassis_mode_e  mode;       //当前底盘控制模式
  chassis_mode_e  last_mode;  //上次底盘控制模式
  
  /* 底盘移动速度相关数据 */
  float           vx;         //底盘前后速度
  float           vy;         //底盘左右速度
  float           vw;         //底盘旋转速度
} chassis_t;


/**
  * @brief     底盘速度分解，计算底盘每个轮子速度
  * @param     vx: 底盘前后速度
  * @param     vy: 底盘左右速度
  * @param     vw: 底盘旋转速度
  * @param     speed[]: 4 个轮子速度数组
  */
void chassis_moto_speed_calc(float vx, float vy, float vw, int16_t speed[]);

#endif
