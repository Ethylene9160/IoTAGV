#ifndef CENTER_MODULE_OSTASK_OLED_UI_H_
#define CENTER_MODULE_OSTASK_OLED_UI_H_

#include "cmsis_os.h"

namespace ostask_oled_ui {

    const osThreadAttr_t task_attributes = {
        .name = "osTaskOLEDUI",
        .stack_size = 256 << 2, // TODO
        .priority = (osPriority_t) osPriorityNormal,
    };

    [[noreturn]] void taskProcedure(void *argument);

} // namespace ostask_oled_ui

#endif // CENTER_MODULE_OSTASK_OLED_UI_H_