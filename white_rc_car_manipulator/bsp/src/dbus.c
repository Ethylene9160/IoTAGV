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

#include "dev.h"
#include "tutorial_lib.h"


extern rc_info_t white_rc;


void dbus_uart_handler(uint8_t *buff) {
    dbus_mesg_handler(&white_rc, buff);
}

void dbus_mesg_handler(rc_info_t *rc, uint8_t *buff) {
    rc->ch1 = (buff[0] | buff[1] << 8) & 0x07FF;
    rc->ch1 -= 1024;
    rc->ch2 = (buff[1] >> 3 | buff[2] << 5) & 0x07FF;
    rc->ch2 -= 1024;
    rc->ch3 = (buff[2] >> 6 | buff[3] << 2 | buff[4] << 10) & 0x07FF;
    rc->ch3 -= 1024;
    rc->ch4 = (buff[4] >> 1 | buff[5] << 7) & 0x07FF;
    rc->ch4 -= 1024;

    /* prevent remote control zero deviation */
    if (rc->ch1 <= 5 && rc->ch1 >= -5)
        rc->ch1 = 0;
    if (rc->ch2 <= 5 && rc->ch2 >= -5)
        rc->ch2 = 0;
    if (rc->ch3 <= 5 && rc->ch3 >= -5)
        rc->ch3 = 0;
    if (rc->ch4 <= 5 && rc->ch4 >= -5)
        rc->ch4 = 0;

    rc->sw1 = ((buff[5] >> 4) & 0x000C) >> 2;
    rc->sw2 = (buff[5] >> 4) & 0x0003;

    if ((abs(rc->ch1) > 660) || (abs(rc->ch2) > 660) || (abs(rc->ch3) > 660) || (abs(rc->ch4) > 660)) {
        memset(rc, 0, sizeof(rc_info_t));
        return;
    }

    rc->mouse.x = buff[6] | (buff[7] << 8); // x axis
    rc->mouse.y = buff[8] | (buff[9] << 8);
    rc->mouse.z = buff[10] | (buff[11] << 8);

    rc->mouse.l = buff[12];
    rc->mouse.r = buff[13];

    rc->key_code = buff[14] | buff[15] << 8; // key borad code
}
