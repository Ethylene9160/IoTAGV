#include "cmsis_os.h"

#include "ostask_chassis.h"


/**
 * @brief 线程注册
 * @note
 *      1. In `sys_start_task` tasks are defined and created for FreeRTOS.
 *      2. `sys_start_task` is called by `librm_lib_1.0.a`, cannot be renamed and must be implemented.
 *      3. `main` is defined in `librm_lib_1.0.a` and cannot be modified, instead, `sys_start_task` is where it all started.
 *      4. Go To Definition to check the usage.
 *      5. [*****] Remember to adjust the stack size for tasks if needed.
 *      6. 注意, 这里是注册用户定义的线程, 在 `librm_lib_1.0.a` 的 `freertos.o` 中已经预先定义了两个任务, 一个是 `StartDefaultTask`, 功能是一秒闪一次绿灯; 另一个是 `unpack_task`, 其中会处理 UART1 和 CAN1 来的消息 (通过信号量, 前者是 2, 后者是 1, 详见对应的注释).
 */
void sys_start_task(void) {
    osThreadDef(chassis, ostask_chassis, osPriorityAboveNormal, 0, 1024); // 注意 Note 第五条, 如果整个卡死了 (比如绿灯都不闪了), 很可能是某个任务爆栈了
    osThreadCreate(osThread(chassis), NULL);
}

/*
 * 关于 Note 第六点, CAN1 在用户侧没有专门的文件, 注释暂时写在这里.
 * `main.o` 的主函数中会调用 `init_can_fx_fifo` (`can_protocol.o`), 其中会使用 `can_callback_reg` (`bsp_can.o`) 将 `can_data_to_fifo` (`can_protocol.o`) 注册为回调函数, 用于将数据入队 (FIFO).
 * CAN1 经过 HAL 处理中断, 收到消息时调用 `HAL_CAN_RxFifo0MsgPendingCallback` (覆写在了 `bsp_can.o` 中), 其中会调用 `can_callback_reg` 设置的回调函数, 并用 `osSignalSet` 给 `unpack_task` 发信号量 1,
 * 该信号会令 `unpack_task` 调用 `can_protocol_unpack` (`can_protocol.o`), 并在其中调用 `Protocol_Unpack`, 再到 `Forward_Packet`, 转发方式详见 `bsp_usart.h` 中 `uart_recv_handler` 的注释, 类似但不完全相同: 第二个字节 0x80 转发 UART1, 0xF0 指令, 没有另一种兼得的情况.
 */

/*
 * 数据帧, 总长 len + 14:
 *      [0]                 0x5A, 同步字节
 *      [1]                 recv_type
 *      [2]                 recv_id
 *      [3]                 send_type
 *      [4]                 send_id
 *      [5], [6]            len, 消息内容部分长度, 低字节在前, 似乎必须小于 386
 *      [7], [8]            似乎是 seq, 低字节在前, 调用一次加一的样子
 *      [9], [10]           cmd_id, 低字节在前
 *      [11]                [0~10] 的 CRC8 结果
 *      [12~(len+11)]       消息内容
 *      [len+12], [len+13]  [0~(len+11)] 的 CRC16 结果 (包括前面的 CRC8 字节), 低字节在前
 * CRC 的具体参数有待计算, 反编译得到了查表法, 目前可以通过 util/crc/ 下的几个 Python 程序计算
 */

/*
 * 关于回调注册, 用户可以使用 `protocol_parser.o` 中的 `module_ack_register` 为特定的 cmd_id 注册回调函数,
 * 数据帧满足一定格式即为指令, `can_protocol_unpack` 和 `pc_unpack_handler` 会对其进行解析并调用 `Module_Cmd_Handle` 执行,
 * 执行的即对应 cmd_id 上注册的回调函数.
 * 默认已经注册的回调函数 (由 `startup.o` 中的 `system_startup` 调用一系列 `mesg_ack_register` 实现) 即 `poten_reg_handler` 等一系列模块的回调, 其参数为消息原字节串及其总长,
 * 在这些函数内部又提取出了数据部分和 `send_id`, 然后就调用了可覆写的 `poten_mesg_handler` 等一系列函数.
 */