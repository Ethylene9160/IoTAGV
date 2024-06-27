# Center Module

### Omitted

TODO

### Notes

1. 高版本编译器下，创建工程时生成的 Middlewares/Third_Party/FreeRTOS/Source/portable/ 下的 RVDS/ARM_CM4F 接口需替换为 GNU 风格的 GCC/ARM_CM4F (从别处找)。故注意每次使用 CubeMX 生成工程后，可能需手动 Rollback 这个目录。

2. controller_module 所用 CAN 参数为 BS1 = 3，BS2 = 5，SJW = ，Time for one Bit 为 1000 ns (TQ = 111.111 ns)。其来源于 APB1 45MHz 五分频后的 9MHz。center_module 所用 F405RGT APB1 最高 42MHz，故调整使其为 36MHz，进行四分频得 9MHz。

3. 发送 CAN 数据帧时的 StdId 在 controller_module 侧似乎做了过滤，但似乎不是在 filter 上，具体没有查明，尝试得 0x0600 至 0x0637 可用（恰好是 0x0600 开始 0 ~ 55，与 can_protocol.o 内 can_protocol_unpack 中循环范围一致，应该有相关性）。
