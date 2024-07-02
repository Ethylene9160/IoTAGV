#include "stm32f10x.h"

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

#include "tiny_io.h"

#include "dwt.h"

#include "uwb_module_config.h"


void InitPeripherals(void) {
    /* SysTick */
    ConfigureSysTick();

    /* GPIO */
    ConfigureGPIO();

    /* SWD */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    /* SPI */
    ConfigureSPI();

    /* USART */
    ConfigureUSART();

    /* Test LED */
    TestLED();
    TurnOffLED(LED_ALL);
}

static uint8 is_initialized = 0;
static uwb_mode_t mode = UNDEFINED;

int main(void) {
    // Initialization
    InitPeripherals();

    // Load the configurations
    // TODO
    mode = JudgeModeFromID(module_config.module_id);

    // Main loop
    while (1) {
        // Command handler
        // TODO 修改 ID 等命令的接收, 若跨类型还要重新 Initialize DW1000.

        if (!is_initialized) {
            if (InitDW1000(mode) != 0) {
                SleepMs(1000);
                continue;
            }
            TurnOnLED((led_t) mode);
            is_initialized = 1;
        }

        // Event handler
        if (mode == ANCHOR) {
            AnchorEventHandler(module_config.module_id);
        } else if (mode == TAG) {
            TagEventHandler(module_config.module_id);
        } else {
            // Do nothing but wait for the module ID to be modified
        }
    }

    return 0;
}
