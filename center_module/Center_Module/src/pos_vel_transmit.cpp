#include "pos_vel_transmit.h"
#include <cstdint>
#include <math.h>

void cart_velocity::update_vel(cart_point& target) {
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