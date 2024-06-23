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
  * V1.0.0      June-26-2018   ric.luo
  * @verbatim
  *********************** (C) COPYRIGHT 2018 DJI **********************
  */

#include "dev.h"
#include "tutorial_lib.h"

#include "chassis_task.h"
#include "math.h"

#define RC_MAX_VALUE      660.0f

/* 底盘移动最大速度，单位是毫米每秒 */
#define MAX_CHASSIS_VX_SPEED 500
#define MAX_CHASSIS_VY_SPEED 500
/* 底盘旋转最大速度，单位是度每秒 */
#define MAX_CHASSIS_VR_SPEED 200

#define VAL_LIMIT(val, min, max)\
if((val) <= (min))\
{\
  (val) = (min);\
}\
else if((val) >= (max))\
{\
  (val) = (max);\
}\


chassis_t      chassis;
rc_info_t      white_rc;

motor_ctrl_t   moto_ctrl;
static int16_t chassis_moto_speed_ref[4];

servo_ctrl_t     servo_ctrl;
wireless_config_t wireless_config;

int16_t manipulator_x = 150;
int16_t manipulator_y = 0;


void manipulator_inverse_calc(int16_t x, int16_t y, uint16_t *s1, uint16_t *s2);


extern sonic_sensor_t sonic_val[4];


void chassis_task(void const *argu) {
    servo_ctrl.s1 = 600;
    servo_ctrl.s2 = 800;
    servo_ctrl_data(1, servo_ctrl);
    task_delay(2000);

    servo_ctrl.s3 = 800;
    servo_ctrl_data(1, servo_ctrl);
//  manipulator_inverse_calc(manipulator_x, manipulator_y, &servo_ctrl.s2, &servo_ctrl.s3);
//  servo_ctrl_data(1, servo_ctrl);

    task_delay(2000);

    wireless_config.dir = 1;

    chassis.mode = CHASSIS_NORMAL;

    while(1) {
//        sonic_mesg_request(1);

        if (white_rc.sw1 == 1) { // 上
            chassis.mode = CHASSIS_NORMAL;
        } else if (white_rc.sw1 == 3) { // 中
            chassis.mode = CHASSIS_STOP;
        } else if (white_rc.sw1 == 2) { // 下
            chassis.mode = CHASSIS_AVOID;
        }

        switch (chassis.mode) {
            case CHASSIS_NORMAL:
                // 遥控器以及鼠标对底盘的控制信息转化为标准单位，平移为 (mm/s) 旋转为 (degree/s)
                chassis.vx = -white_rc.ch1 / RC_MAX_VALUE * MAX_CHASSIS_VX_SPEED;
                chassis.vy = white_rc.ch2 / RC_MAX_VALUE * MAX_CHASSIS_VY_SPEED;
                chassis.vw = white_rc.ch3 / RC_MAX_VALUE * MAX_CHASSIS_VR_SPEED;
                break;

            case CHASSIS_AVOID:
//                chassis.vx = 0;
//                if (sonic_val[0].distance <= 700 && sonic_val[0].distance != 0) {
//                    chassis.vy = 0;
//                    chassis.vw = 200;
//                } else {
//                    chassis.vy = 150;
//                    chassis.vw = 0;
//                }

                chassis.vy = 150;
                chassis.vw = 0;

                break;

            case CHASSIS_STOP:
                chassis.vy = 0;
                chassis.vx = 0;
                chassis.vw = 0;

                servo_ctrl.s1 += white_rc.ch1 / 10.0;
                manipulator_x += white_rc.ch4 / 20.0;
                manipulator_y += white_rc.ch2 / 20.0;

                // 坐标限定
                VAL_LIMIT(manipulator_x, 120, 200);
                VAL_LIMIT(manipulator_y, -50, 50);
                manipulator_inverse_calc(manipulator_x, manipulator_y, &servo_ctrl.s3, &servo_ctrl.s2);

                // 舵机限定
                VAL_LIMIT(servo_ctrl.s1, 600,  900);
//                VAL_LIMIT(servo_ctrl.s2, 500,  900);
//                VAL_LIMIT(servo_ctrl.s3, 500,  900);
                servo_ctrl_data(1, servo_ctrl);
                break;

            default:
                chassis.vy = 0;
                chassis.vx = 0;
                chassis.vw = 0;
                break;
        }

        if (chassis.mode == CHASSIS_RELAX) {
            moto_ctrl.m1_enable = 0;
            moto_ctrl.m2_enable = 0;
            moto_ctrl.m3_enable = 0;
            moto_ctrl.m4_enable = 0;
        } else {
            moto_ctrl.m1_enable = 1;
            moto_ctrl.m2_enable = 1;
            moto_ctrl.m3_enable = 1;
            moto_ctrl.m4_enable = 1;

            chassis_moto_speed_calc(chassis.vx, chassis.vy, chassis.vw, chassis_moto_speed_ref);
            moto_ctrl.m1_rpm = chassis_moto_speed_ref[0];
            moto_ctrl.m2_rpm = chassis_moto_speed_ref[1];
            moto_ctrl.m3_rpm = chassis_moto_speed_ref[2];
            moto_ctrl.m4_rpm = chassis_moto_speed_ref[3];
        }

        motor_ctrl_data(1, moto_ctrl);
        task_delay(50);
    }
}

/* 1 臂长 (mm) */
#define ARM1_LENGTH     120
/* 2 臂长 (mm) */
#define ARM2_LENGTH     120
/* 舵机最大角度 (degree)  */
#define SERVO_MAX_ANGLE 280

void manipulator_inverse_calc(int16_t x, int16_t y, uint16_t *s1, uint16_t *s2) {
    static int16_t r1 = ARM1_LENGTH;
    static int16_t r2 = ARM2_LENGTH;

    double angle1, a1, b1;
    double angle2, a2, b2;

    a1 = r1 * r1 + x * x + y * y - r2 * r2;
    b1 = 2 * r1 * sqrt(x * x + y * y);
    a2 = r2 * r2 + x * x + y * y - r1 * r1;
    b2 = 2 * r2 * sqrt(x * x + y * y);

    angle1 = (acos(a1/b1) + atan(1.0*y/x)) * 57.3;
    angle2 = (3.14 - acos(a2/b2) + atan(1.0*y/x)) * 57.3;

    // 角度限定
//    VAL_LIMIT(angle1, 15, 90);
//    VAL_LIMIT(angle2, 90, 180);

    *s1 = 2000 * angle1 / SERVO_MAX_ANGLE + 500;  // 将所有舵机初始位置标到 0，S1 舵机接大摆杆，舵机臂臂朝内侧平行
    *s2 = 2000 * (180.0 - angle2) / SERVO_MAX_ANGLE + 500;// S2 接小摆杆，舵机臂外侧平行， 爪子标到 0，在最大张开角度安装舵机臂
}


/* 底盘轮距 (mm) */
#define WHEELTRACK     250
/* 底盘轴距 (mm) */
#define WHEELBASE      250
/* 云台偏移 (mm) */
#define GIMBAL_OFFSET  0
/* 角度转弧度系数 */
#define RADIAN_COEF    57.3f
/* 底盘轮子周长 (mm) */
#define PERIMETER      76
/* 底盘电机减速比 */
#define CHASSIS_DECELE_RATIO (1.0f/36.0f)
/* 单个电机速度极限，单位是每分钟转 */
#define MAX_WHEEL_RPM        15000   // 15000 rpm = 528 mm/s
//MAX_WHEEL_RPM / 60 * PERIMETER * CHASSIS_DECELE_RATIO

/**
  * @param 输入参数1: vx左右平移速度(mm/s)，右为正方向
  *        输入参数2: vy前后平移速度(mm/s)，前为正方向
  *        输入参数3: vw底盘旋转速度(degree/s)，逆时针为正方向
  *        输入参数4: speed[] 4个电机转速(rpm)
  * @note  下面是电机轮子编号，左上角为0号
  * @map   1 %++++++% 0
               ++++
               ++++
           2 %++++++% 3
  */
void chassis_moto_speed_calc(float vx, float vy, float vw, int16_t speed[]) {
    static float rotate_ratio_f = ((WHEELBASE+WHEELTRACK)/2.0f - GIMBAL_OFFSET)/RADIAN_COEF;
    static float rotate_ratio_b = ((WHEELBASE+WHEELTRACK)/2.0f + GIMBAL_OFFSET)/RADIAN_COEF;
    static float wheel_rpm_ratio = 60.0f/(PERIMETER*CHASSIS_DECELE_RATIO);

    int16_t wheel_rpm[4];
    float max = 0;

    VAL_LIMIT(vx, -MAX_CHASSIS_VX_SPEED, MAX_CHASSIS_VX_SPEED);  //mm/s
    VAL_LIMIT(vy, -MAX_CHASSIS_VY_SPEED, MAX_CHASSIS_VY_SPEED);  //mm/s
    VAL_LIMIT(vw, -MAX_CHASSIS_VR_SPEED, MAX_CHASSIS_VR_SPEED);  //deg/s

    wheel_rpm[0] = (-vx - vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[1] = (-vx + vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[2] = (+vx + vy + vw * rotate_ratio_b) * wheel_rpm_ratio;
    wheel_rpm[3] = (+vx - vy + vw * rotate_ratio_b) * wheel_rpm_ratio;

    // find max item
    for (uint8_t i = 0; i < 4; i++) {
        if (abs(wheel_rpm[i]) > max)
            max = abs(wheel_rpm[i]);
    }

    // equal proportion
    if (max > MAX_WHEEL_RPM) {
        float rate = MAX_WHEEL_RPM / max;
        for (uint8_t i = 0; i < 4; i++)
            wheel_rpm[i] *= rate;
    }
    memcpy(speed, wheel_rpm, 4*sizeof(int16_t));
}

void class_test_task(void const *argu) {
    while (1) {
        task_delay(500);
    }
}
