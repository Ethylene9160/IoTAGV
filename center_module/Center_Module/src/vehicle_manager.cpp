#include "vehicle_manager.h"
#include <cmath>

vehicle_controller::vehicle_controller(cart_point target_point)
    : target_point(target_point) {
    self_vel.v_cons = 1.0f;
    self_vel.vx = self_vel.vy = self_vel.w = 0.0f;
    self_point = {0.0f, 0.0f}; // Initializing Pose
}

void vehicle_controller::tick() {
    float total_weight_x = 0.0f;
    float total_weight_y = 0.0f;
    float bias_x = 0.0f;
    float bias_y = 0.0f;

    for (const auto& vehicle : vehicle_position) {
        _update_self_vel(vehicle.second, bias_x, bias_y, total_weight_x, total_weight_y);
    }

    if (total_weight_x > 0) {
        bias_x /= total_weight_x;
    }
    if (total_weight_y > 0) {
        bias_y /= total_weight_y;
    }

    self_vel.vx = self_vel.v_cons + bias_x;
    self_vel.vy = self_vel.v_cons + bias_y;

    self_point.x += self_vel.vx;
    self_point.y += self_vel.vy;
}

void vehicle_controller::_update_self_vel(const cart_point& obstacle, float& bias_x, float& bias_y, float& total_weight_x, float& total_weight_y) {
    float dx = obstacle.x - self_point.x;
    float dy = obstacle.y - self_point.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.001f) {
        return;
    }

    float weight = 1.0f / distance;

    bias_x -= weight * dx;
    bias_y -= weight * dy;

    total_weight_x += weight;
    total_weight_y += weight;
}

void vehicle_controller::set_self_point(const cart_point& point) {
    self_point = point;
}

cart_point vehicle_controller::get_self_point() const {
    return self_point;
}

void vehicle_controller::set_self_velocity(const cart_velocity& velocity) {
    self_vel = velocity;
}

cart_velocity vehicle_controller::get_self_velocity() const {
    return self_vel;
}
