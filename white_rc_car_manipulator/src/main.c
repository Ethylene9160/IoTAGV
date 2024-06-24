#include "cmsis_os.h" // for `osThreadDef` and `osThreadCreate`

#include "ostask_chassis.h"


/**
 * @brief 线程注册
 * @note
 *      1. In `sys_start_task` tasks are defined and created for FreeRTOS.
 *      2. `sys_start_task` is called by `librm_lib_1.0.a`, cannot be renamed and must be implemented.
 *      3. `main` is defined in `librm_lib_1.0.a` and cannot be modified, instead, `sys_start_task` is where it all started.
 *      4. Go To Definition to check the usage.
 *      5. Remember to adjust the stack size for tasks if needed.
 */
void sys_start_task(void) {
    osThreadDef(chassis, ostask_chassis, osPriorityAboveNormal, 0, 128);
    osThreadCreate(osThread(chassis), NULL);
}
