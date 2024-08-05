#ifndef KALMAN_H
#define KALMAN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float Q_angle;
    float Q_gyro;
    float R_angle;
    float x_bias;
    float angle;
    float P[2][2];
} Kalman;

void Kalman_Init(Kalman *kf);
float Kalman_Update(Kalman *kf, float newAngle, float newRate, float dt);

#ifdef __cplusplus
}
#endif
#endif

// #ifndef KALMAN_H
// #define KALMAN_H
// #ifdef __cplusplus
// extern "C" {
// #endif
//
//     typedef struct {
//         float Angle; // 最优角度
//         float Gyro; // 最优角速度
//
//         float Q_bias, Angle_err;
//
//         float PCt_0, PCt_1, E;
//         float K_0, K_1, t_0, t_1;
//
//         float Pdot[4];
//         float P[2][2]; // 误差协方差矩阵
//     } Kalman;
//
//     void Kalman_Init(Kalman *kf);
//     float Kalman_Update(Kalman *kf, float angle_a, float gyro, float dt);
//
// #ifdef __cplusplus
// }
// #endif
// #endif
