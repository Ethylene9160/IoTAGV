#include <ios>

#include "main.h"

#include "cmsis_os.h"

#include "ostask_controller_module_port.h"
#include "ostask_uwb_module_port.h"
#include "ostask_usart_transformer.h"
#include "ostask_test_task.h"

#include "port_uart.h"
#include "vehicle_controller.h"

#include <memory>

#include "ostask_vehicle_controller.h"

/* ** Example Begin ** */
const osThreadAttr_t test_task_attributes = {
    .name = "osTaskControllerModulePort",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

[[noreturn]] void testTaskProcedure(void *argument) {
    auto *v = new msgs::Value<uint32_t>(1000);
    ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_EXPIRED_TIME, v));
    osDelay(1000);
    while (true) {
        msgs::Twist2D* t1 = new msgs::Twist2D(20.0f, 0.0f, 0.0f);
        msgs::Twist2D* t2 = new msgs::Twist2D(0.0f, 20.0f, 0.0f);
        msgs::Twist2D* t3 = new msgs::Twist2D(-20.0f, 0.0f, 0.0f);
        msgs::Twist2D* t4 = new msgs::Twist2D(0.0f, -20.0f, 0.0f);
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, t1));
        osDelay(2000);
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, t2));
        osDelay(2000);
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, t3));
        osDelay(2000);
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, t4));
        osDelay(2000);
    }
}
/* ** Example End ** */

void startThreads() {

    // 设置起点和终点坐标
    cart_point _start{1.6f,2.0f};
    cart_point _terminal{1.2f,4.0f};

    // auto vehicle_controller_ptr = std::make_unique<vehicle_controller>(0, _start, _terminal);
    auto* vehicle_controller_ptr = new vehicle_controller(0x80, _start, _terminal);

    // 随机放入一些障碍物
    // cart_point ob1{1.0f, 0.0f};
    // cart_point ob2{3.0f, 3.0f};
    // vehicle_controller_ptr->push_back(2, ob1);
    // vehicle_controller_ptr->push_back(3, ob2);

    // controller module port thread
    osThreadNew(ostask_controller_module_port::taskProcedure, nullptr, &ostask_controller_module_port::task_attributes);

    // test task: 走正方形。
    // osThreadNew(testTaskProcedure, nullptr, &test_task_attributes); // ** Example: 超时时间设为 1s, 每隔 2s 动作一次 (2s 内前 1s 动作, 后 1s 输出 "Expired." 并停止), 绕逆时针方形轨迹. **

    // vehicle controller thread
    osThreadNew(ostask_vehicle_controller::taskProcedure, vehicle_controller_ptr, &ostask_vehicle_controller::task_attributes);

    // test task: 随机发送障碍物位置，看速度是否正确。
    // osThreadNew(ostask_test_task::taskProcedure, 0, &ostask_test_task::task_attributes);
}

/*
 * Usage Memo:
 *      1. #include "ostask_controller_module_port.h"
 *      2. 使用 ostask_controller_module_port::pushCommand(<CMD_TYPE>, <CMD_DATA>) 推送命令:
 *          <CMD_TYPE> 为命令类型, 见 ostask_controller_module_port.h 中的宏定义和 controller_module 的 ostask_chassis.c 中的注释;
 *          <CMD_DATA> 为命令内容, 见 msgs.h 中的定义, 目前就 Example 中使用的两种命令和参数.
 */