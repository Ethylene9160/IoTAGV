#include "vehicle_manager.h"
#include <cmath>
#include <random>

vehicle_controller::vehicle_controller(uint16_t self_id, cart_point current_point, cart_point target_point)
    : target_point(target_point), self_id(self_id), self_point(current_point) {
    self_vel.v_cons = 1.0f;
    self_vel.vx = self_vel.vy = self_vel.w = 0.0f;
    //self_point = { 0.0f, 0.0f };
    const osMutexAttr_t Controller_MutexAttr = {
        .name = "Controller_Mutex"
      };
    this->vehicle_controller_mutex = osMutexNew(&Controller_MutexAttr);
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

    for (const auto& vehicle : vehicle_position) {
        if (_is_obstacle_near(vehicle.second, self_vel.vx, self_vel.vy)) {
            _add_noise_to_velocity(self_vel.vx, self_vel.vy);
            break;
        }
    }

    self_point.x += self_vel.vx;
    self_point.y += self_vel.vy;
}

bool vehicle_controller::_is_obstacle_near(const cart_point& obstacle, float vx, float vy) {
    float dx = obstacle.x - self_point.x;
    float dy = obstacle.y - self_point.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    return distance < 1.0f && (dx * vx + dy * vy) > 0;
}

void vehicle_controller::_add_noise_to_velocity(float& vx, float& vy) {
    static std::default_random_engine generator;
    static std::normal_distribution<float> distribution(0.0f, 0.1f);

    vx += distribution(generator);
    vy += distribution(generator);
}

void vehicle_controller::_update_self_vel(
    const cart_point& obstacle,
    float& bias_x,
    float& bias_y,
    float& total_weight_x,
    float& total_weight_y)
{
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

void vehicle_controller::push(uint16_t id, cart_point point) {
    auto status = osMutexAcquire(this->vehicle_controller_mutex, osWaitForever);
    if (status == osOK) {
        if (id == self_id) {
            set_self_point(point);  // 更新自己的cart_point
        } else {
            auto it = this->vehicle_position.find(id);
            if (it == this->vehicle_position.end()) {
                // 新来的，添加到哈希表
                this->vehicle_position[id] = point;
            } else {
                // 更新它的point
                it->second = point;
            }
        }
        osMutexRelease(this->vehicle_controller_mutex);
    }
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

void vehicle_controller::add_obstacle(uint16_t id, const cart_point& point) {
    if (id == self_id) {
        set_self_point(point);
    }
    else {
        vehicle_position[id] = point;
    }
}

void tag_receive_broad(uint8_t* buffer) {

}