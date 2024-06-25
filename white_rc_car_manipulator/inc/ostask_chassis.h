#ifndef CHASSIS_TASK_H_
#define CHASSIS_TASK_H_

#include "stdint.h" // for intx_t


#define RC_MAX_VALUE 3000.0f
#define MAX_CHASSIS_VX_SPEED 500 // 底盘前后移动最大速度 (mm/s)
#define MAX_CHASSIS_VY_SPEED 500 // 底盘左右移动最大速度 (mm/s)
#define MAX_CHASSIS_VR_SPEED 200 // 底盘旋转最大角速度 (deg/s)

#define WHEELTRACK 250 // 底盘轮距 (mm)
#define WHEELBASE 250 // 底盘轴距 (mm)
#define GIMBAL_OFFSET 0 // 云台偏移 (mm)
#define RADIAN_COEF 0.017453f // 角度转弧度系数, pi / 180
#define PERIMETER 76 // 底盘轮子周长 (mm)
#define CHASSIS_DECELE_RATIO (1.0f / 36.0f) // 底盘电机减速比
#define MAX_WHEEL_RPM 15000 // 单个电机速度极限 (rpm), 15000 rpm = 528 mm/s

#define VAL_LIMIT(val, min, max) if ((val) <= (min)) { (val) = (min); } else if ((val) >= (max)) { (val) = (max); }


/**
  * @brief 底盘控制模式
  */
typedef enum {
    CHASSIS_NORMAL,         // 正常
    CHASSIS_STOP            // 停止
} chassis_mode_e;

/**
  * @brief 底盘控制数据
  */
typedef struct {
    chassis_mode_e mode;    // 控制模式
    float vx;               // x 速度
    float vy;               // y 速度
    float vw;               // 角速度
} chassis_t;


/**
 * @brief 底盘控制线程主程序
 * @param argu
 */
void ostask_chassis(void const *argu);

#endif