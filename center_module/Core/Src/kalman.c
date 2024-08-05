#include "Kalman.h"

void Kalman_Init(Kalman *kf) {
    kf->Q_angle = 0.001f;
    kf->Q_gyro = 0.003f;
    kf->R_angle = 0.5f;
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
