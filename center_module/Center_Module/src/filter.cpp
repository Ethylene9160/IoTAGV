#include "filter.h"

namespace center_filter {
    // Base class.
    Filter::Filter(float init_val) : _val(init_val) {}

    Filter::~Filter() {}

    float Filter::value() const {
        return _val;
    }

    // LBF class implementation
    LBF::LBF(float alpha, float init_val) : Filter(init_val), alpha(alpha) {}

    void LBF::filter(float value) {
        _val = alpha * value + (1.0f - alpha) * _val;
    }

    // Average filter
    AverageFilter::AverageFilter(uint32_t len, float init_val)
        : Filter(init_val), index(0), max_len(len), buffer(len, init_val) {}

    AverageFilter::~AverageFilter() {}

    void AverageFilter::filter(float value) {
        _val -= buffer[++index] / max_len;
        _val += value / max_len;
        buffer[index] = value;
        if (index == max_len - 1)
            index = -1;
    }

    // Kalman class implementation
    Kalman::Kalman(float Q_angle, float Q_gyro, float R_angle, float bias, float init_val)
        : Filter(init_val), Q_angle(Q_angle), Q_gyro(Q_gyro), R_angle(R_angle), x_bias(bias) {
        P[0][0] = 1.0f;
        P[0][1] = 0.0f;
        P[1][0] = 0.0f;
        P[1][1] = 1.0f;
    }

    void Kalman::predict(float newRate, float dt) {
        _val += dt * (newRate - x_bias);
        P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
        P[0][1] -= dt * P[1][1];
        P[1][0] -= dt * P[1][1];
        P[1][1] += Q_gyro * dt;
    }

    void Kalman::filter(float value) {
        _update(value);
    }

    void Kalman::_update(float newAngle) {
        float S = P[0][0] + R_angle;
        float K[2]; // Kalman gain - This is a 2x1 vector
        K[0] = P[0][0] / S;
        K[1] = P[1][0] / S;

        float y = newAngle - _val; // Angle difference
        _val += K[0] * y;
        x_bias += K[1] * y;

        float P00_temp = P[0][0];
        float P01_temp = P[0][1];

        P[0][0] -= K[0] * P00_temp;
        P[0][1] -= K[0] * P01_temp;
        P[1][0] -= K[1] * P00_temp;
        P[1][1] -= K[1] * P01_temp;
    }
}
