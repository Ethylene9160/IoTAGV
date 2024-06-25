#include "ostask_chassis.h"

#include "stdio.h"

#include "cmsis_os.h"
#include "librm.h"


rc_info_t white_rc;


/**
  * @brief 底盘速度分解, 计算底盘每个轮子速度
  * @param vx 左右平移速度 (mm/s), 右为正方向
  * @param vy 前后平移速度 (mm/s), 前为正方向
  * @param vw 底盘旋转速度 (degree/s), 逆时针为正方向
  * @param speed[] 四个电机转速 (rpm)
  * @note 数组索引中左前轮电机为 0, 逆时针顺序编号 0 ~ 3; 对应实机电调 SET 时的序号则是逆时针 1 ~ 4, 右前为 1.
  */
void calc_motor_velocities(float vx, float vy, float vw, int16_t *speed) {
    static float rotate_ratio_f = ((WHEELBASE + WHEELTRACK) / 2.0f - GIMBAL_OFFSET) * RADIAN_COEF;
    static float rotate_ratio_b = ((WHEELBASE + WHEELTRACK) / 2.0f + GIMBAL_OFFSET) * RADIAN_COEF;
    static float wheel_rpm_ratio = 60.0f/(PERIMETER*CHASSIS_DECELE_RATIO);

    int16_t wheel_rpm[4];
    float max = 0;

    VAL_LIMIT(vx, -MAX_CHASSIS_VX_SPEED, MAX_CHASSIS_VX_SPEED);
    VAL_LIMIT(vy, -MAX_CHASSIS_VY_SPEED, MAX_CHASSIS_VY_SPEED);
    VAL_LIMIT(vw, -MAX_CHASSIS_VR_SPEED, MAX_CHASSIS_VR_SPEED);

    wheel_rpm[0] = (-vx - vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[1] = (-vx + vy + vw * rotate_ratio_f) * wheel_rpm_ratio;
    wheel_rpm[2] = (+vx + vy + vw * rotate_ratio_b) * wheel_rpm_ratio;
    wheel_rpm[3] = (+vx - vy + vw * rotate_ratio_b) * wheel_rpm_ratio;

    // find max item
    for (uint8_t i = 0; i < 4; i++) {
        if (abs(wheel_rpm[i]) > max) {
            max = abs(wheel_rpm[i]);
        }
    }

    // equal proportion
    if (max > MAX_WHEEL_RPM) {
        float rate = MAX_WHEEL_RPM / max;
        for (uint8_t i = 0; i < 4; i++)
            wheel_rpm[i] *= rate;
    }
    memcpy(speed, wheel_rpm, 4 * sizeof(int16_t));
}

void test_reg_handler(uint8_t *p_buf, uint16_t len) {
    uint16_t data_len = len - 14; // *((uint16_t *) (p_buf + 5));
    uint8_t recv_data[data_len];
    memcpy(recv_data, p_buf + 12, data_len);
    char str[50];
    sprintf(str, "Value = %d, Len = %d.\n", (int) recv_data[0], data_len);
    bsp_uart1_sendbuf((uint8_t *) str, strlen(str));
}

void ostask_chassis(void const *argu) {
    task_delay(1000);

    // Examples (下次 Commit 即删除):
    // 5A RT RI ST SI  LEN   SEQ   CMD  C8 DA  C16

    // 执行指令 (0x5A, 0xF0, 0x00, ... 或 0x5A, 0xFF, 0xFF, ...):
    // 注册回调:
    module_ack_register(0x0080u, test_reg_handler);
    // 数据为 00:
    // 5A F0 00 01 01 01 00 00 00 80 00 4E 00 99 0B
    // 5A FF FF 01 01 01 00 00 00 80 00 9A 00 2B 6B
    // 数据为 01:
    // 5A F0 00 01 01 01 00 01 00 80 00 C1 01 3F 11
    // 5A FF FF 01 01 01 00 01 00 80 00 15 01 8D 71

    // 发送数据转发回来 (0x5A, 0x80, 0x00, ...):
    // 5A 80 00 01 01 01 00 00 00 80 00 B3 00 E1 AE
//    uint8_t data_0[1] = {0x00};
//    send_pc_pack_cmd(1u, 1u, 0x0080u, data_0, 1);

    while (1) {
        chassis_mode_e mode;
        float vx, vy, w;

        rc_info_t rc;
        taskENTER_CRITICAL();
        memcpy(&rc, &white_rc, sizeof(white_rc));
        taskEXIT_CRITICAL();

        if (rc.sw1 == 3) { // 中
            mode = CHASSIS_NORMAL;
        } else {
            mode = CHASSIS_STOP;
        }

        switch (mode) {
            case CHASSIS_NORMAL:
                // 控制信息转化为标准单位，平移为 (mm/s) 旋转为 (degree/s)
                vx = (float) -rc.ch1 / RC_MAX_VALUE * MAX_CHASSIS_VX_SPEED;
                vy = (float) rc.ch2 / RC_MAX_VALUE * MAX_CHASSIS_VY_SPEED;
                w = (float) rc.ch3 / RC_MAX_VALUE * MAX_CHASSIS_VR_SPEED;
                break;

            default:
                vy = 0;
                vx = 0;
                w = 0;
                break;
        }

        motor_ctrl_t moto_ctrl;
        moto_ctrl.m1_enable = 1;
        moto_ctrl.m2_enable = 1;
        moto_ctrl.m3_enable = 1;
        moto_ctrl.m4_enable = 1;

        int16_t chassis_moto_speed_ref[4];
        calc_motor_velocities(vx, vy, w, chassis_moto_speed_ref);
        moto_ctrl.m1_rpm = chassis_moto_speed_ref[0];
        moto_ctrl.m2_rpm = chassis_moto_speed_ref[1];
        moto_ctrl.m3_rpm = chassis_moto_speed_ref[2];
        moto_ctrl.m4_rpm = chassis_moto_speed_ref[3];

        motor_ctrl_data(1, moto_ctrl);

        task_delay(50);
    }
}
