#include <ios>
#include <memory>

#include "cmsis_os.h"

#include "main.h"

#include "port_uart.h"
#include "vehicle_controller.h"

#include "ostask_controller_module_port.h"
#include "ostask_vehicle_controller.h"
#include "ostask_oled_ui.h"
#include "ostask_remote_control.h"
#include "ostask_uwb_module_port.h"
#include "ostask_usart_transformer.h"
#include "ostask_mpu6050.h"
#include "mpu.h"

typedef struct {
    cart_point start;
    cart_point terminal;
    uint16_t id;
} _vehicle_config;

#define V2

#ifdef V0
_vehicle_config vehicle_config_default = {
        .start = {1.6f, 2.0f},
        .terminal = {1.0f, 4.5f},
        .id = 0x80
};
#endif

#ifdef V1
_vehicle_config vehicle_config_default = {
        .start = {1.6f, 2.0f},
        // .terminal = {3.0f, 1.5f},
        .terminal = {1.6f, 5.5f},
        .id = 0x81
};
#endif

#ifdef V2
_vehicle_config vehicle_config_default = {
    .start = {1.6f, 2.0f},
    .terminal = {-0.3f, 1.5f},
    .id = 0x82
};
#endif

void startThreads() {
    auto* vehicle_controller_ptr = new vehicle_controller(vehicle_config_default.id, vehicle_config_default.start, vehicle_config_default.terminal);
    vehicle_controller_ptr->stop();
    // 下发 CAN 指令到 controller_module 的任务
    osThreadNew(ostask_controller_module_port::taskProcedure, nullptr, &ostask_controller_module_port::task_attributes);

    ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, new msgs::Twist2D(0.0f, 0.0f, 0.0f)));

    // 处理算法控制的任务
    osThreadNew(ostask_vehicle_controller::taskProcedure, vehicle_controller_ptr, &ostask_vehicle_controller::task_attributes);

    // 接收上位机指令控制的任务
    osThreadNew(ostask_remote_control::taskProcedure, vehicle_controller_ptr, &ostask_remote_control::task_attributes);

    // OLED GUI 显示任务
    osThreadNew(ostask_oled_ui::taskProcedure, vehicle_controller_ptr, &ostask_oled_ui::task_attributes);

}

/*
 * Usage Memo:
 *      1. #include "ostask_controller_module_port.h"
 *      2. 使用 ostask_controller_module_port::pushCommand(<CMD_TYPE>, <CMD_DATA>) 推送命令:
 *          <CMD_TYPE> 为命令类型, 见 ostask_controller_module_port.h 中的宏定义和 controller_module 的 ostask_chassis.c 中的注释;
 *          <CMD_DATA> 为命令内容, 见 msgs.h 中的定义, 目前就 Example 中使用的两种命令和参数.
 */