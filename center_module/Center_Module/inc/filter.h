#ifndef CENTER_MODULE_FILTER_H
#define CENTER_MODULE_FILTER_H

#include <cstdint>
#include <vector>

namespace center_filter {
    class Filter {
    public:
        Filter(float init_val);
        virtual ~Filter();
        virtual void filter(float value) = 0;
        float value() const;

    protected:
        float _val;
    };

    class LBF : public Filter {
    public:
        LBF(float alpha, float init_val = 0.0f);
        void filter(float value) override;

    private:
        float alpha;
    };

    class AverageFilter : public Filter {
    public:
        AverageFilter(uint32_t len, float init_val = 0.0f);
        virtual ~AverageFilter();
        void filter(float value) override;

    private:
        uint32_t index;
        uint32_t max_len;
        std::vector<float> buffer;
    };

    class Kalman : public Filter {
    public:
        Kalman(
            float Q_angle = 0.001f,
            float Q_gyro = 0.003f,
            float R_angle = 0.5f,
            float bias = 0.0f,
            float init_val = 0.0f);

        void predict(float newRate, float dt);
        void filter(float value) override;

    private:
        float x_bias; // The gyro bias calculated by the Kalman filter - part of the 2x1 state vector
        float P[2][2]; // Error covariance matrix - This is a 2x2 matrix
        float Q_angle; // Process noise variance for the accelerometer
        float Q_gyro; // Process noise variance for the gyro bias
        float R_angle; // Measurement noise variance

        void _update(float newAngle);
    };
}

#endif //CENTER_MODULE_FILTER_H
