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
  *             June-25-2024
  * @verbatim
  *********************** (C) COPYRIGHT 2018 DJI **********************
  */

#include "tutorial_lib.h"


uint8_t motor_id;
motor_sensor_t motor_val;


/**
 * @note TODO: `motor_mesg_handler` 并没有在 `tutorial_lib.o` 中被作为外部符号调用, 不确定其它地方有没有, 至少单步调试不经过该函数. 删去不影响功能, 暂时保留.
 */
void motor_mesg_handler(uint8_t send_id, motor_sensor_t *data) {
    motor_id = send_id;

    switch (send_id) {
        case 1:
            memcpy(&motor_val, data, sizeof(motor_sensor_t));
            break;
    }
}
