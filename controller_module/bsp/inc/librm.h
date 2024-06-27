/**
 * 该文件包含且应该仅包含 `librm_lib_1.0.a` 中声明的一些符号和外部符号.
 * 该文件已经包含 `tutorial_lib.h` (其中包含了 `dev.h`) `bsp_usart.h` 等, 用户代码中欲使用该静态库中的内容, 应当仅包含该文件; bsp 内部的实现建议包含细分的头文件而非该文件.
 * 从静态库中引出新的内容, 也直接添加头文件和可能需要的实现, 包含进本文件即可.
 */

#ifndef LIBRM_H
#define LIBRM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dev.h"
#include "tutorial_lib.h"
#include "bsp_usart.h"
#include "protocol_parser.h"
#include "bsp_can.h"

#ifdef __cplusplus
};
#endif

#endif