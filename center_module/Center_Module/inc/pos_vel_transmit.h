#ifndef VELOCITY_BIAS_H
#define VELOCITY_BIAS_H
#include <math.h>
#include <vector>

namespace pos_vel_trainsmit {
    struct polar_point {
        float r;
        float t;
    };
    struct cart_point {
        float x;
        float y;
    };

    class cart_velocity {
    public:

        void update_vel(std::vector<cart_point>& targets) {
            for(cart_point& p:targets) {
                update_vel(p);
            }
        }

        void update_vel(cart_point& target) {
            static float _w_k = 1.0f;
            static float _w_bias = 0.05f;

            float _delta_x = this->p.x - target.x;
            float _delta_y = this->p.y - target.y;

            float _k = 1.0f/((_delta_x * _delta_x) + (_delta_y * _delta_y));
            float _theta = atan2(_delta_y, _delta_x);

            this->vx += _k * _delta_x;
            this->vy += _k * _delta_y;
            this->w += _w_k * _theta + _w_bias;
        }

    public:
        float vx;
        float vy;
        float w;
        cart_point p;
    };

}

#endif //VELOCITY_BIAS_H
