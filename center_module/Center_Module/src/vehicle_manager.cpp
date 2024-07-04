#include "vehicle_manager.h"
#include <math.h>

float cart_velocity::v_cons = 2.0f;

void cart_velocity::_update_vel(cart_point& target) {
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

car_msg_controller::car_msg_controller(cart_point target_point) : target_point(target_point) {}

void car_msg_controller::tick() {
    float _delta_x = this->target_point.x - this->self_point.x;
    float _delta_y = this->target_point.y - this->self_point.y;
    float _theta = atan2f(_delta_y, _delta_x);
    float vx = car_msg_controller::v_cons * cosf(_theta);
    float vy = car_msg_controller::v_cons * sinf(_theta);
    // this->vel = {vx, vy};
    memcpy(&this->vel.vx, &vx, 4);
    memcpy(&this->vel.vy, &vy, 4);
}

void car_msg_controller::_update_self_vel() {

}

void cart_velocity::update_vel() {
    for(auto& it : vehicle_position){
        _update_vel(it.second);
    }
}

// float cart_velocity::LINEAR_VEL = 2.5f;

void tag_receive_broad(uint8_t *buffer){
    uint16_t source_id = 0;
    uint16_t target_id = 0;
    memcpy(&source_id, &buffer[1], 2);
    memcpy(&target_id, &buffer[3], 2);
    if (source_id <= 0x0FFF || target_id != 0xFFFF) {
        //不是来自tag的信息，也不是广播包
        return;
    }
    float x = 0;
    float y = 0;
    memcpy(&x, &buffer[8], 4);
    memcpy(&y, &buffer[12], 4);

    cart_point p = {x, y};
    auto it = vehicle_position.find(source_id);
    if(it != vehicle_position.end()){
        it->second.x = p.x;
        it->second.y = p.y;
    }else{
        vehicle_position[source_id] = p;
    }
}