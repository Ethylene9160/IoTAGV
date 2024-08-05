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
