# IoTAGV

![Static STM](https://img.shields.io/badge/STM-32-green)
![Static C++](https://img.shields.io/badge/C++-11-blue)
![Static License](https://img.shields.io/badge/License-NO-orange)
![Static Date](https://img.shields.io/badge/Date-2024--08--03-lightgrey)

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

## Memo

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

## 使用手册

### 单车清单

* STM32F425IIB6
* STM32F405RGT6
* DWM1000套装
* 基站2个（天线）
* 车3个。
* 遥控器两个及以上
* STLink至少拿5个
* **充电宝**尽可能多
* 2.5mm螺丝刀*2
* 2.0mm螺丝刀*2
* 

# Contributors

<a href="https://github.com/Ethylene9160/IoTAGV/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=Ethylene9160/IoTAGV" />
</a>

# 附：接线说明

## UWB Module



## Center Module

* STLink2烧录

| Pin  | Describe |
| :--: | :------: |
| PA13 |  SWDIO   |
| PA14 |  SWCLK   |

* 与UWB Module的通信：使用*USART1*

| Pin  | Describe  |
| :--: | :-------: |
| PA9  | USART1_TX |
| PA10 | USART1_RX |

* 与上位机的串口通信：使用*USART2*

| Pin  | Describe  |
| :--: | :-------: |
| PA2  | USART2_TX |
| PA3  | USART2_RX |

* 与屏幕的通信：使用I2C1

* 与IMU的通信：使用`I2C2`

## Controller Module

直插can总线通讯线。
