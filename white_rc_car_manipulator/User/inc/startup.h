/****************************************************************************
 *  Copyright (C) 2018 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of聽
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.聽 See the
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
  * V1.0.0      June-26-2018   ric.luo
  * @verbatim
  *********************** (C) COPYRIGHT 2018 DJI **********************
  */ 
 


#ifndef __START_UP_H__
#define __START_UP_H__

#include "sys.h"
#include "cmsis_os.h"
#include "class_task.h"

#define USER_TASK1   class_test_task
#define USER_TASK2   chassis_task
//#define USER_TASK3
//#define USER_TASK4
//#define USER_TASK5


void sys_start_task(void);

#endif
