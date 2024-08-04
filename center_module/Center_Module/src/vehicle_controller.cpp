#include "vehicle_controller.h"
#include <cmath>
#include <random>

#include "usart.h"

float vehicle_controller::v_cons = 48.5f;
float vehicle_controller::v_k = 16.5f;
float vehicle_controller::collision_radius = 0.30f;
float vehicle_controller::large_bias = 100.0f;  // 用于处理重合时的很大偏置


vehicle_controller::vehicle_controller(
    uint16_t self_id,
    cart_point current_point,
    cart_point target_point
): target_point(target_point), self_id(self_id), self_point(current_point), isTerminal(0) { // TODO: 之后改为初始默认停止 (isTerminal = 1)，由控制器控制启动
    self_vel.vx = self_vel.vy = self_vel.w = 0.0f;
    const osMutexAttr_t Controller_MutexAttr = {
        .name = "Controller_Mutex"
    };
    this->vehicle_controller_mutex = osMutexNew(&Controller_MutexAttr);
}

void vehicle_controller::tick() {
    static float _v_norm = 18.0f;
    float total_weight_x = 0.0f;
    float total_weight_y = 0.0f;
    float bias_x = 0.0f;
    float bias_y = 0.0f;

    float _dx = target_point.x - self_point.x;
    float _dy = target_point.y - self_point.y;
    float _d = std::sqrt(_dx * _dx + _dy * _dy);

    for (const auto &vehicle: vehicle_position) {
        // char buffer[64];
        // int len = sprintf(buffer, "obstacle:%d,  %.2f, %.2f, self: %.2f, %.2f\n", vehicle.first, vehicle.second.x, vehicle.second.y, self_point.x, self_point.y);
        // HAL_UART_Transmit(&huart2, (uint8_t *)buffer, len, 0xffff);
        _update_self_vel(vehicle.second, bias_x, bias_y, total_weight_x, total_weight_y);
    }

    // if (total_weight_x > 0) {
    //     bias_x /= total_weight_x;
    // }
    // if (total_weight_y > 0) {
    //     bias_y /= total_weight_y;
    // }

    self_vel.vx = vehicle_controller::v_cons * _dx/_d + bias_x * vehicle_controller::v_k;
    self_vel.vy = vehicle_controller::v_cons * _dy/_d + bias_y * vehicle_controller::v_k;

    float _v_square = std::sqrt(self_vel.vx * self_vel.vx + self_vel.vy * self_vel.vy);
    if (_v_square > 0.1f && _v_square < _v_norm) {
        float _k = _v_norm / _v_square;
        self_vel.vx *= _k;
        self_vel.vy *= _k;
    }

    // for (const auto &vehicle: vehicle_position) {
    //     if (_is_obstacle_near(vehicle.second, self_vel.vx, self_vel.vy)) {
    //         _add_noise_to_velocity(self_vel.vx, self_vel.vy);
    //         break;
    //     }
    // }
}

inline bool vehicle_controller::_is_obstacle_near(const cart_point &obstacle, float vx, float vy) {
    float dx = obstacle.x - self_point.x;
    float dy = obstacle.y - self_point.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    return distance < 0.2f && (dx * vx + dy * vy) > 0;
}

void vehicle_controller::_add_noise_to_velocity(float &vx, float &vy) {
    static std::default_random_engine generator;
    static std::normal_distribution<float> distribution(0.0f, 1.0f);

    vx += distribution(generator) * 0.02;
    vy += distribution(generator) * 0.02;
}

inline void vehicle_controller::_update_self_vel(
    const cart_point &obstacle,
    float &bias_x,
    float &bias_y,
    float &total_weight_x,
    float &total_weight_y) {
    // judge nan
    if (std::isnan(obstacle.x) || std::isnan(obstacle.y)) {
        return;
    }
    float dx = obstacle.x - self_point.x;
    float dy = obstacle.y - self_point.y;

    float d2 = dx * dx + dy * dy;
    float distance = std::sqrt(d2) - vehicle_controller::collision_radius * 2.0f;
    if (distance < 0.02f) {
        distance = 0.02f;
    }
    // d2 = distance * distance;
    // if(distance < 0.0f) {
    // bias_x -= vehicle_controller::large_bias * dx;
    //     bias_y -= vehicle_controller::large_bias * dy;
    //     return;
    // }

    if (distance > 0.98f) {
        return;
    }

    float weight = 1.0f / distance;

    bias_x -= weight * dx;
    bias_y -= weight * dy;

    total_weight_x += weight;
    total_weight_y += weight;
}

void vehicle_controller::push_back(uint16_t id, cart_point point) {
    auto status = osMutexAcquire(this->vehicle_controller_mutex, osWaitForever);
    if (status == osOK) {
        if (id == self_id) {
            point.x = (self_point.x + point.x) / 2.0f;
            point.y = (self_point.y + point.y) / 2.0f;
            set_self_point(point); // 更新自己的cart_point

            if(this->_is_near_target(this->target_point)) {
                isTerminal = true;
            }
        } else {
            auto it = this->vehicle_position.find(id);
            if (it == this->vehicle_position.end()) {
                // 新来的，添加到哈希表
                this->vehicle_position[id] = point;
            } else {
                // first order filter.
                float _x = (point.x+it->second.x)/2.0f;
                float _y = (point.y+it->second.y)/2.0f;
                it->second.x = _x;
                it->second.y = _y;
            }
        }
    }
    osMutexRelease(this->vehicle_controller_mutex);
}

inline void vehicle_controller::set_self_point(const cart_point &point) {
    self_point = point;
}

cart_point vehicle_controller::get_self_point() const {
    return self_point;
}

void vehicle_controller::set_self_velocity(const cart_velocity &velocity) {
    self_vel = velocity;
}

bool vehicle_controller::is_terminal() {
    return isTerminal;
}

cart_velocity vehicle_controller::get_self_velocity() const {
    if (isTerminal) {
        return {0.0f, 0.0f, 0.0f};
    }
    return self_vel;
}

//<<<<<<< master
//void vehicle_controller::set_terminated(bool is_terminated) {
  //  this->isTerminal = is_terminated;
//}

//#define EPS 1e-5

//void vehicle_controller::set_target_point(const cart_point _target_point) {
  //  if (std::abs(this->target_point.x - _target_point.x) < EPS && abs(this->target_point.y - _target_point.y) < EPS) {
    //    return;
    //}
    //this->target_point = _target_point;
    //this->set_terminated(false);
//}
//=======
void vehicle_controller::set_target_point(const cart_point &point) {
    if (std::abs(point.x - target_point.x) < 0.05f && std::abs(point.y - target_point.y) < 0.05f) {
        return;
    }

    target_point = point;
    if(this->_is_near_target(target_point)) {
        isTerminal = true;
    }else {
        isTerminal = false;
    }
}

cart_point vehicle_controller::get_target_point() const {
    return target_point;
}

void vehicle_controller::stop() {
    self_vel.vx = 0.0f;
    self_vel.vy = 0.0f;
    isTerminal = 1;
}

void vehicle_controller::start() {
    isTerminal = 0;
}

bool vehicle_controller::_is_near_target(const cart_point &target) {
    return (std::abs(target.x - self_point.x) < 0.16f) && (std::abs(target.y - self_point.y) < 0.16f);
}

uint16_t vehicle_controller::get_self_id() const {
    return this->self_id;
}

//>>>>>>> ethy_branch
