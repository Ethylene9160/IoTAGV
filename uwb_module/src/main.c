#include "stm32f10x.h"

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

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


static uint8_t is_initialized = 0;

int main(void) {
    // Initialization
    InitPeripherals();

    // Load the configurations
    // TODO, 读配置, 修改全局 module_config

    // Main loop
    while (1) {
        // Command handler
        // TODO 修改 ID 等命令的接收, 若跨类型还要重新 Initialize DW1000.

        if (!is_initialized) {
            if (Initialize() != 0) {
                SleepMs(1000); // TODO: 若编写了 Command Handler 则须换非阻塞
                continue;
            }
            is_initialized = 1;
        }

        // Event handler
        EventHandler();
    }

    return 0;
}
