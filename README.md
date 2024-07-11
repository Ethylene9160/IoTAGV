# IoTAGV

![Static STM](https://img.shields.io/badge/STM-32-green)
![Static C++](https://img.shields.io/badge/C++-11-blue)
![Static License](https://img.shields.io/badge/License-NO-orange)
![Static Date](https://img.shields.io/badge/Date-2024--07--10-lightgrey)

应用 UWB 通信技术，构建去中心化的多车定位及车间通信系统，实现多车协同运动与自动防撞。

## 演示

### 仿真

![image_sim](README.assets/vehicle_simulation.gif)

### 实机

* 静态障碍物躲避

![static_sim](README.assets/static_sim.gif)

* 动态障碍物躲避

![dynamic_sim](README.assets/dynamic_sim.gif)

* 复杂障碍物躲避

![sophisticated_sim](README.assets/sophisticated_sim.gif)

### 文件结构

### 系统架构

TODO

### 文件结构

```
center_module: 主控模块
TODO.
```

### Memo

模块间连接与常用功能引脚接线快速查询：

* STM32 SWD Wires

| Pin | Desc |
| :--: | :--: |
| PA13 | SWDIO |
| PA14 | SWCLK |

* Center Module UART1 (with UWB Module)

| Pin | Desc |
| :--: | :--: |
| PA9 | USART1_TX |
| PA10 | USART1_RX |

* Center Module UART2 (with PC, debug)

| Pin | Desc |
| :--: | :--: |
| PA2 | USART2_TX |
| PA3 | USART2_RX |

### 使用手册

TODO

### Contributors

<a href="https://github.com/Ethylene9160/IoTAGV/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=Ethylene9160/IoTAGV" />
</a>
