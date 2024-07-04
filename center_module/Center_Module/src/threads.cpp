#include "main.h"

#include "cmsis_os.h"

#include "ostask_controller_module_port.h"
#include "ostask_uwb_module_port.h"
#include "ostask_usart_transformer.h"

#include "port_uart.h"
#include "vehicle_manager.h"

#include <memory>

#include "ostask_vehicle_controller.h"

/* ** Example Begin ** */
const osThreadAttr_t test_task_attributes = {
    .name = "osTaskControllerModulePort",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

[[noreturn]] void testTaskProcedure(void *argument) {
    ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_EXPIRED_TIME, msgs::Value<uint32_t>(1000)));
    osDelay(1000);

    while (true) {
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, msgs::Twist2D(20.0f, 0.0f, 0.0f)));
        osDelay(2000);
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, msgs::Twist2D(0.0f, 20.0f, 0.0f)));
        osDelay(2000);
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, msgs::Twist2D(-20.0f, 0.0f, 0.0f)));
        osDelay(2000);
        ostask_controller_module_port::pushCommand(msgs::Command(CTRL_CMD_SET_TWIST, msgs::Twist2D(0.0f, -20.0f, 0.0f)));
        osDelay(2000);
    }
}
/* ** Example End ** */

void startThreads() {

    auto port_usart_ptr = std::make_unique<PortUART>(0, huart2);

    auto vehicle_controller_ptr = std::make_unique<vehicle_controller>(cart_point{0.0f, 0.0f});

    // controller module port thread
    osThreadNew(ostask_controller_module_port::taskProcedure, nullptr, &ostask_controller_module_port::task_attributes);

    osThreadNew(testTaskProcedure, nullptr, &test_task_attributes); // ** Example: 超时时间设为 1s, 每隔 2s 动作一次 (2s 内前 1s 动作, 后 1s 输出 "Expired." 并停止), 绕逆时针方形轨迹. **

    // usart transformer thread
    osThreadNew(ostask_usart_transformer::taskProcedure, (void*)port_usart_ptr.get(), &ostask_usart_transformer::task_attributes);

    // uwb module port thread
    osThreadNew(ostask_uwb_module_port::taskProcedure, (void*)port_usart_ptr.get(), &ostask_uwb_module_port::task_attributes);

    // vehicle controller thread
    osThreadNew(ostask_vehicle_controller::taskProcedure, (void*)vehicle_controller_ptr.get(), &ostask_vehicle_controller::task_attributes);
}

/*
 * Usage Memo:
 *      1. #include "ostask_controller_module_port.h"
 *      2. 使用 ostask_controller_module_port::pushCommand(<CMD_TYPE>, <CMD_DATA>) 推送命令:
 *          <CMD_TYPE> 为命令类型, 见 ostask_controller_module_port.h 中的宏定义和 controller_module 的 ostask_chassis.c 中的注释;
 *          <CMD_DATA> 为命令内容, 见 msgs.h 中的定义, 目前就 Example 中使用的两种命令和参数.
 */