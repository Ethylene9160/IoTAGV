#ifndef CENTER_MODULE_POS_VEL_TRANSMIT_H_
#define CENTER_MODULE_POS_VEL_TRANSMIT_H_

#include "uwb_tool.h"

class cart_velocity {
public:
    void update_vel(cart_point& target);

public:
    float vx;
    float vy;
    float w;
    cart_point p;
};

#endif