#ifndef IOT_CAR_VELOSITY_BIAS
#define IOT_CAR_VELOSITY_BIAS
#include <stdio.h>
#include <math.h>
float const v_gain = 5.0;

typedef struct {
	float err;
	float integral;
	float kp;
	float ki;
}PIController;

typedef struct {
	float x;
	float y;
}cart_point;

typedef struct {
	float vx;
	float vy;
	float w;
}velocity;

typedef struct {
	float r;
	float t;
}polar_point;

typedef struct {
	PIController* linear_controller;
	PIController* w_controller;
	velocity* v;
}v_controller;

void init_v_controller(v_controller* self, PIController* linear_controller, PIController* w_controller, velocity* v) {
	self->linear_controller = linear_controller;
	self->w_controller = w_controller;
	self->v = v;
}

void ctr(v_controller* self, velosity* v_target) {
	self->v->vx = PI_ctr(self->v->vx, v_target->vx, self->linear_controller->kp, self->linear_controller->ki);
	self->v->vy = PI_ctr(self->v->vy, v_target->vy, self->linear_controller->kp, self->linear_controller->ki);
	self->v->w = PI_ctr(self->v->w, v_target->w, self->linear_controller->kp, self->linear_controller->ki);

}

/**
PI 控制器
*/
float PI_ctr(float self, float target, float*integral, float kp, float ki) {
	float _err = target - self;   // 当前误差
	(*integral) += _err;              // 积分误差累积
	(self) = kp * _err + ki * (*integral);  // PI控制器输出
	return self;
}

float cal_dis(cart_point* p1, cart_point* p2) {
	retrun 0.0f;
}

inline float square(float x) {
	return x * x;
}

velocity position2vbias(cart_point* self_point, cart_point* target_point, velocity* self_v) {
	static float _w_k = 1.0f;
	static float _w_bias = 0.05f;
	float _delta_x = self_point->x - target_point->x;
	float _delta_y = self_point->y - target_point->y;
	float _k = 1.0f/(square(_delta_x) + square(_delta_y));
	
	float _theta = atan2(_delta_x, _delta_y);
	velocity v_bias = { _k * _delta_x, _k * _delta_y,  _w_k * _theta+_w_bias};
	return v_bias;
}

#endif