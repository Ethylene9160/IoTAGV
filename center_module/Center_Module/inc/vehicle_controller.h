#ifndef CENTER_MODULE_VEHICLE_MANAGER_H_
#define CENTER_MODULE_VEHICLE_MANAGER_H_

#include <map>
#include <cstdint>
#include "cmsis_os.h"
#include <memory.h>

#include "filter.h"


/**
 * Remote control command type.
 */
enum RC_CMD {
    RC_CMD_NONE = 0,
    RC_CMD_SET_TARGET_POSITION = 1,
    RC_CMD_SET_VELOCITY = 2,
    RC_CMD_PAUSE = 3,
    RC_CMD_RESUME = 4,
    RC_CMD_TURN_LEFT_A_BIT = 5,
    RC_CMD_TURN_RIGHT_A_BIT = 6,
    RC_CMD_STOP_TURNING = 7
};

/**
 * Polar position.
 */
typedef struct {
    float r;
    float t;
} polar_point;

/**
 * Cartesian position.
 */
typedef struct {
    float x;
    float y;
} cart_point;

typedef struct {
    float vx;
    float vy;
    float w;
} cart_velocity;

typedef struct {
    cart_point current_point;
    cart_point target_point;
} vehicle_info;


class vehicle_controller {
public:
    static float v_cons;
    static float v_k;
    static float kp;
    static float ki;
    static float kd;
    static float collision_radius;
    static float large_bias;

    vehicle_controller(uint16_t self_id, cart_point current_point, cart_point target_point);

    void tick();

    void set_self_point(const cart_point &point);

    cart_point get_self_point() const;

    void set_self_velocity(const cart_velocity &velocity);

    cart_velocity get_self_velocity() const;

    void set_target_point(const cart_point &point);

    cart_point get_target_point() const;

    void stop();

    void start();

    uint16_t get_self_id() const;

    /**
     * push the obstacle to the vehicle_position
     * @param id id of the obstacle
     * @param point position of the obstacle
     */
    void push_back(uint16_t id, cart_point point);

    bool is_terminal();

    float get_delta_alpha();

    void set_init_alpha(float init_alpha);

    void set_current_alpha(float alpha);

    float get_init_alpha();

    float get_current_alpha();

    void process_remote_command(uint8_t cmd_type, float opt1, float opt2);

    ~vehicle_controller();

private:
    uint16_t self_id;
    cart_point target_point;
    cart_point self_point;
    cart_velocity self_vel;
    bool isTerminal;

    float init_alpha;
    float current_alpha;

    center_filter::Filter *filter1;
    center_filter::Filter *filter2;

    std::map<uint16_t, cart_point> vehicle_position;

    osMutexId_t vehicle_controller_mutex;

    void _update_self_vel(const cart_point &obstacle, float &bias_x, float &bias_y, float &total_weight_x, float &total_weight_y);

    bool _is_obstacle_near(const cart_point &obstacle, float vx, float vy);

    void _add_noise_to_velocity(float &vx, float &vy);

    bool _is_near_target(const cart_point& target);

    void _update_w();

};

#endif