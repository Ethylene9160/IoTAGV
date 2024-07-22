#include "ostask_chassis.h"

#include "stdio.h"

#include "cmsis_os.h"
#include "librm.h"


rc_info_t white_rc;

chassis_cartesian_velocity_stamped_t velocity;

uint32_t expired_time = 1000;


/**
  * @brief 底盘速度分解, 计算底盘每个轮子速度
  * @param vx 左右平移速度 (mm/s), 右为正方向
  * @param vy 前后平移速度 (mm/s), 前为正方向
  * @param vw 底盘旋转速度 (degree/s), 逆时针为正方向
  * @param speed[] 四个电机转速 (rpm)
  * @note 数组索引中右前轮电机为 0, 逆时针顺序编号 0 ~ 3; 对应实机电调 SET 时的序号则是逆时针 1 ~ 4, 同样右前为 1.
  */
void calc_motor_velocities(float vx, float vy, float vw, int16_t *speed) {
    static float rotate_ratio_f = ((WHEELBASE + WHEELTRACK) / 2.0f - GIMBAL_OFFSET) * RADIAN_COEF;
    static float rotate_ratio_b = ((WHEELBASE + WHEELTRACK) / 2.0f + GIMBAL_OFFSET) * RADIAN_COEF;
    static float wheel_rpm_ratio = 60.0f / (PERIMETER * CHASSIS_DECELE_RATIO);

    int16_t wheel_rpm[4];
    float max = 0;

    VAL_LIMIT(vx, -MAX_CHASSIS_VX_SPEED, MAX_CHASSIS_VX_SPEED);
    VAL_LIMIT(vy, -MAX_CHASSIS_VY_SPEED, MAX_CHASSIS_VY_SPEED);
    VAL_LIMIT(vw, -MAX_CHASSIS_VR_SPEED, MAX_CHASSIS_VR_SPEED);

    wheel_rpm[0] = (- vx - vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[1] = (+ vx - vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[2] = (+ vx + vy + vw * rotate_ratio_b) * wheel_rpm_ratio;
    wheel_rpm[3] = (- vx + vy + vw * rotate_ratio_b) * wheel_rpm_ratio;

    for (uint8_t i = 0; i < 4; i++) {
        if (abs(wheel_rpm[i]) > max) {
            max = abs(wheel_rpm[i]);
        }
    }

    if (max > MAX_WHEEL_RPM) {
        float rate = MAX_WHEEL_RPM / max;
        for (uint8_t i = 0; i < 4; i++)
            wheel_rpm[i] *= rate;
    }
    memcpy(speed, wheel_rpm, sizeof(wheel_rpm));
}

uint32_t get_stamp(void) {
    return (uint32_t) xTaskGetTickCount();
}

uint8_t is_expired(uint32_t stamp, uint32_t time_limit) {
    return get_stamp() - stamp > pdMS_TO_TICKS(time_limit) ? 1 : 0;
}

/*
 * 用户指令:
 *    cmd_id    data format                                     desc
 *    0x0100    vx (float:32), vy (float:32), w (float:32)      控制底盘速度. vx, vy, w 单位分别为 mm/s 和 deg/s.
 *                                                              e.g. 5A F0 00 00 00 0C 00 00 00 00 01 0B 00 00 20 41 00 00 00 00 00 00 00 00 A0 BD (vx = 10 mm/s)
 *                                                                   5A F0 00 00 00 0C 00 00 00 00 01 0B 00 00 20 C1 00 00 00 00 00 00 00 00 77 79 (vx = -10 mm/s)
 *                                                              使用 `cartesian_velocity_msg_gen.py` 和 `gen_msg.py` 可以手动生成数据帧.
 *    0x0101    expired_time (uint32_t:32)                      设置底盘速度控制超时时间, 单位为 ms.
 *                                                              e.g. 5A F0 00 00 00 04 00 00 00 01 01 6E E8 03 00 00 B7 B9 (set as 1000 ms)
 *                                                                   5A F0 00 00 00 04 00 00 00 01 01 6E D0 07 00 00 FC 73 (set as 2000 ms)
 * P.S. 数据按照架构默认的大小端模式进行传输, 即此处 float 等数据的高字节在前.
 */

void chassis_cartesian_velocity_control_reg_handler(uint8_t *p_buf, uint16_t len) {
    uint16_t len_data = len - 14;
    uint8_t data[len_data];
    memcpy(data, p_buf + 12, len_data);

    // Parse
    taskENTER_CRITICAL();
    velocity.stamp = get_stamp();
    memcpy(&velocity.vx, data, 4);
    memcpy(&velocity.vy, data + 4, 4);
    memcpy(&velocity.w, data + 8, 4);
    taskEXIT_CRITICAL();

    // Debug Info
    char str[128];
    sprintf(str, "Received: vx = %.2f (mm/s), vy = %.2f (mm/s), w = %.2f (deg/s).\n", *((float *) (data)), *((float *) (data + 4)), *((float *) (data + 8)));
    bsp_uart1_sendbuf((uint8_t *) str, strlen(str));
}

void chassis_set_expired_time_reg_handler(uint8_t *p_buf, uint16_t len) {
    uint16_t len_data = len - 14;
    uint8_t data[len_data];
    memcpy(data, p_buf + 12, len_data);

    taskENTER_CRITICAL();
    memcpy(&expired_time, data, 4);
    taskEXIT_CRITICAL();

    char str[128];
    sprintf(str, "Set expired time: %lu ms.\n", *((uint32_t *) data));
    bsp_uart1_sendbuf((uint8_t *) str, strlen(str));
}

void ostask_chassis(void const *argu) {
    osDelay(100);

    chassis_mode_e mode;
    rc_info_t rc;

    velocity.stamp = 0; // 为 0 代表已重置
    velocity.vx = 0;
    velocity.vy = 0;
    velocity.w = 0;

    module_ack_register(0x0100u, chassis_cartesian_velocity_control_reg_handler);
    module_ack_register(0x0101u, chassis_set_expired_time_reg_handler);

    while (1) {
        taskENTER_CRITICAL();
        memcpy(&rc, &white_rc, sizeof(white_rc));
        taskEXIT_CRITICAL();

        mode = CHASSIS_CAN_CONTROL;
        // if (rc.sw1 == 1) { // 上, 停止
        //     mode = CHASSIS_STOP;
        // } else if (rc.sw1 == 3) { // 中, 指令控制
        //     mode = CHASSIS_CAN_CONTROL;
        // } else { // 下, 遥控器控制
        //     mode = CHASSIS_REMOTE_CONTROL;
        // }

        switch (mode) {
            case CHASSIS_REMOTE_CONTROL: {
                // 控制信息转化为标准单位，平移为 (mm/s) 旋转为 (degree/s)
                float vx = (float) rc.ch2 / RC_MAX_VALUE * MAX_CHASSIS_VX_SPEED;
                float vy = (float) -rc.ch1 / RC_MAX_VALUE * MAX_CHASSIS_VY_SPEED;
                float w = (float) rc.ch3 / RC_MAX_VALUE * MAX_CHASSIS_VR_SPEED;
                taskENTER_CRITICAL();
                velocity.stamp = 0;
                velocity.vx = vx;
                velocity.vy = vy;
                velocity.w = w;
                taskEXIT_CRITICAL();
                break;
            }

            case CHASSIS_CAN_CONTROL: {
                if (velocity.stamp && is_expired(velocity.stamp, expired_time)) {
                    taskENTER_CRITICAL();
                    velocity.stamp = 0;
                    velocity.vx = 0;
                    velocity.vy = 0;
                    velocity.w = 0;
                    taskEXIT_CRITICAL();

                    char str[10];
                    sprintf(str, "Expired.\n");
                    bsp_uart1_sendbuf((uint8_t *) str, strlen(str));
                }
                break;
            }

            default: {
                taskENTER_CRITICAL();
                velocity.stamp = 0;
                velocity.vx = 0;
                velocity.vy = 0;
                velocity.w = 0;
                taskEXIT_CRITICAL();
                break;
            }
        }

        motor_ctrl_t moto_ctrl;
        moto_ctrl.m1_enable = 1;
        moto_ctrl.m2_enable = 1;
        moto_ctrl.m3_enable = 1;
        moto_ctrl.m4_enable = 1;

        taskENTER_CRITICAL();
        float vx = velocity.vx;
        float vy = velocity.vy;
        float w = velocity.w;
        taskEXIT_CRITICAL();

        int16_t chassis_moto_speed_ref[4];
        calc_motor_velocities(vx, vy, w, chassis_moto_speed_ref);
        moto_ctrl.m1_rpm = chassis_moto_speed_ref[0];
        moto_ctrl.m2_rpm = chassis_moto_speed_ref[1];
        moto_ctrl.m3_rpm = chassis_moto_speed_ref[2];
        moto_ctrl.m4_rpm = chassis_moto_speed_ref[3];
        motor_ctrl_data(1, moto_ctrl);

        osDelay(50);
    }
}
