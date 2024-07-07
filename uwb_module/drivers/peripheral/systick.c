#include "systick.h"


extern __IO uint64_t systick_ms;

uint32_t GetSystickMs(void) {
    return systick_ms;
}

void SleepMs(unsigned int time_ms) {
    unsigned long end = GetSystickMs() + time_ms;
    while ((signed long)(GetSystickMs() - end) <= 0);
}

int ConfigureSysTick(void) {
    if (SysTick_Config(SystemCoreClock / 1000)) { // CLOCKS_PER_SEC = 1000
        /* Capture error */
        while (1);
    }
    NVIC_SetPriority(SysTick_IRQn, 15); // TODO, raw = 6
    return 0;
}
