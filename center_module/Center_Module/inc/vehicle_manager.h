#ifndef VEHICLE_MANAGER_H
#define VEHICLE_MANAGER_H

#include <map>
#include "uwb_tool.h"
#include <cstdint>
#include <memory.h>

inline std::map<uint16_t, cart_point> vehicle_position;

void tag_receive_broad(uint8_t *buffer);

class cart_velocity {
public:
    void update_vel();

public:
    // static float LINEAR_VEL;
    float vx;
    float vy;
    float w;
    cart_point p;
    static float v_cons;

    void _update_vel(cart_point& target);
};

class car_msg_controller {
public:
    explicit car_msg_controller(cart_point target_point);

    void tick();

private:
    cart_point target_point;
    cart_point self_point;
    cart_velocity vel;
    cart_velocity self_vel;
    static float v_cons;

    void _update_self_vel();

};

#endif //VEHICLE_MANAGER_H
