# IoTAGV

V2V.

### 文件结构



## GPIO多功能引脚

参考官方手册，如下功能应当对应引脚如下：

* STLink烧录

|   PA14   |    SWCLW  |
| :--: | :--: |
|   PA13   |   SWDIO   |

* USART1

|   PA10   | USART1_RX  |
| :--: | :--: |
|   PA9   |   USART1_TX   |
|   PA8   |   USART1_CK   |

## 开发计划

### 通信

* `uwb_module`与`center_module`

预使用USART通信，每个元素是一个8位的无符号整型`uint16_t`变量。通信格式如下：


### 数据帧格式



* `center_module`与`controller_module`

该模块的通信需要与`controller_module`保持一致。



## 使用手册

