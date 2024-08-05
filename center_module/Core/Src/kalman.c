#include "Kalman.h"

void Kalman_Init(Kalman *kf) {
    kf->Q_angle = 0.001f;
    kf->Q_gyro = 0.003f;
    kf->R_angle = 0.03f;
    kf->x_bias = 0.0f;
    kf->angle = 0.0f;
    kf->P[0][0] = 1.0f;
    kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;
    kf->P[1][1] = 1.0f;
}

float Kalman_Update(Kalman *kf, float newAngle, float newRate, float dt) {
    // Predict
    kf->angle += dt * (newRate - kf->x_bias);
    kf->P[0][0] += dt * (dt * kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_gyro * dt;

    // Update
    float S = kf->P[0][0] + kf->R_angle;
    float K[2]; // Kalman gain - This is a 2x1 vector
    K[0] = kf->P[0][0] / S;
    K[1] = kf->P[1][0] / S;

    float y = newAngle - kf->angle; // Angle difference
    kf->angle += K[0] * y;
    kf->x_bias += K[1] * y;

    float P00_temp = kf->P[0][0];
    float P01_temp = kf->P[0][1];

    kf->P[0][0] -= K[0] * P00_temp;
    kf->P[0][1] -= K[0] * P01_temp;
    kf->P[1][0] -= K[1] * P00_temp;
    kf->P[1][1] -= K[1] * P01_temp;

    return kf->angle;
}

// #include "Kalman.h"
//
// static const float Q_angle=0.001f;  		//角度过程噪声协方差
// static const float Q_gyro=0.003f;				//角速度过程噪声协方差
// static const float R_angle=0.5f;				//预测模型噪声协方差
// static const char  C_0 = 1;
//
// void Kalman_Init(Kalman *kf) {
//     kf->P[0][0] = 1.0f;
//     kf->P[0][1] = 0.0f;
//     kf->P[1][0] = 0.0f;
//     kf->P[1][1] = 1.0f;
// }
//
// float Kalman_Update(Kalman *kf, float angle_a, float gyro, float dt) {
//     kf->Gyro = gyro / 16.384f;
//     kf->Angle += dt * (kf->Gyro - kf->Q_bias); // 先验估计
//
//     kf->Pdot[0] = Q_angle - kf->P[0][1] - kf->P[1][0];
//     kf->Pdot[1] = -(kf->P[1][1]);
//     kf->Pdot[2] = -(kf->P[1][1]);
//     kf->Pdot[3] = Q_gyro;
//
//     kf->P[0][0] += kf->Pdot[0] * dt;   // Pk-先验估计误差协方差微分的积分
//     kf->P[0][1] += kf->Pdot[1] * dt;   // =先验估计误差协方差
//     kf->P[1][0] += kf->Pdot[2] * dt;
//     kf->P[1][1] += kf->Pdot[3] * dt;
//
//     kf->Angle_err = angle_a - kf->Angle;
//
//     kf->PCt_0 = C_0 * kf->P[0][0];
//     kf->PCt_1 = C_0 * kf->P[1][0];
//
//     kf->E = R_angle + C_0 * kf->PCt_0;
//
//     kf->K_0 = kf->PCt_0 / kf->E;
//     kf->K_1 = kf->PCt_1 / kf->E;
//
//     kf->t_0 = kf->PCt_0;
//     kf->t_1 = C_0 * kf->P[0][1];
//
//     kf->P[0][0] -= kf->K_0 * kf->t_0;
//
//     //后验估计误差协方差
//     kf->P[0][1] -= kf->K_0 * kf->t_1;
//     kf->P[1][0] -= kf->K_1 * kf->t_0;
//     kf->P[1][1] -= kf->K_1 * kf->t_1;
//
//     kf->Angle += kf->K_0 * kf->Angle_err;
//     //后验估计
//     kf->Q_bias  += kf->K_1 * kf->Angle_err; //后验估计
//     kf->Gyro   = kf->Gyro - kf->Q_bias;
//     //输出值(后验估计)角
//     return  kf->Angle;
// }
