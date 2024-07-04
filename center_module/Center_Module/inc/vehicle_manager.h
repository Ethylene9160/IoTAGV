#ifndef VEHICLE_MANAGER_H
#define VEHICLE_MANAGER_H

#include <map>
// #include "uwb_tool.h"
#include <cstdint>
#include <memory.h>

typedef struct {
    float x;
    float y;
} cart_point;

typedef struct {
    float vx;
    float vy;
    float w;
    float v_cons;
} cart_velocity;

inline std::map<uint16_t, cart_point> vehicle_position;

void tag_receive_broad(uint8_t *buffer);

class vehicle_controller {
public:
    explicit vehicle_controller(cart_point target_point);

    void tick();

    void set_self_point(const cart_point& point);
    cart_point get_self_point() const;

    void set_self_velocity(const cart_velocity& velocity);
    cart_velocity get_self_velocity() const;

private:
    cart_point target_point;
    cart_point self_point;
    cart_velocity self_vel;

    void _update_self_vel(const cart_point& obstacle, float& bias_x, float& bias_y, float& total_weight_x, float& total_weight_y);
};

#endif //VEHICLE_MANAGER_H
