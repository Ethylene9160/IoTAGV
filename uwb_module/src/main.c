#include "stm32f10x.h"

//#include <stdio.h>
//#include <string.h>
//#include <stdint.h>
//#include <math.h>

#include "uwb_module_config.h"

//#include "deca_device_api.h"
//#include "deca_regs.h"
//#include "deca_sleep.h"
//#include "trilateration.h"

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"


void initPeripherals(void) {
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
//    Frame_Init((FrameContext*) &frame_ctx);

    /* Test LED */
    TestLED();
}


int main(void) {
    // Initialization
    initPeripherals();

    // Load the configurations
    // TODO

    // Initialize DW1000
    // TODO

    // Main loop
    while (1) {
        // Command handler
        // TODO 修改 ID 等命令的接收
        debug_printf("Module ID: %d\n", config.module_id); // TEST

        // Event handler
        if (config.module_id >= 0x0000 && config.module_id <= 0x0FFF) {
            // Anchor
            // TODO
        } else if (config.module_id >= 0x1000 && config.module_id <= 0x1FFF) {
            // Tag
            // TODO
        } else {
            // Do nothing but wait for the module ID to be modified
        }

        SleepMs(1000);
    }

    return 0;
}
