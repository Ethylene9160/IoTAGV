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

#include "startup.h"

//#ifdef USER_TASK1
//extern osThreadId task1_t;
//#endif
//#ifdef USER_TASK2
//extern osThreadId task2_t;
//#endif
//#ifdef USER_TASK3
//extern osThreadId task3_t;
//#endif
//#ifdef USER_TASK4
//extern osThreadId task4_t;
//#endif
//#ifdef USER_TASK5
//extern osThreadId task5_t;
//#endif

extern osThreadId task1_t, task2_t;


void sys_start_task(void) {
    osThreadDef(ostask1, USER_TASK1, osPriorityAboveNormal, 0, 128);
    task1_t = osThreadCreate(osThread(ostask1), NULL);

    osThreadDef(ostask2, USER_TASK2, osPriorityAboveNormal, 0, 128);
    task2_t = osThreadCreate(osThread(ostask2), NULL);

//#ifdef USER_TASK1
//    osThreadDef(ostask1, USER_TASK1, osPriorityAboveNormal, 0, 128);
//    task1_t = osThreadCreate(osThread(ostask1), NULL);
//#endif
//
//#ifdef USER_TASK2
//    osThreadDef(ostask2, USER_TASK2, osPriorityAboveNormal, 0, 128);
//    task2_t = osThreadCreate(osThread(ostask2), NULL);
//#endif
//
//#ifdef USER_TASK3
//    osThreadDef(ostask3, USER_TASK3, osPriorityNormal, 0, 128);
//    task3_t = osThreadCreate(osThread(ostask3), NULL);
//#endif
//
//#ifdef USER_TASK4
//    osThreadDef(ostask4, USER_TASK4, osPriorityNormal, 0, 128);
//    task4_t = osThreadCreate(osThread(ostask4), NULL);
//#endif
//
//#ifdef USER_TASK5
//    osThreadDef(ostask5, USER_TASK5, osPriorityNormal, 0, 128);
//    task5_t = osThreadCreate(osThread(ostask5), NULL);
//#endif
}
