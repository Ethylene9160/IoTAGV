#include "stm32f10x.h"

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

#include "dwt.h"
#include "adc.h"

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

void USART1_IRQHandler(void) {
    static uint8_t u1_rx_buffer[35];
    static uint8_t U1_RX_LEN = 12;
    static uint8_t u1_index = 0;
    static uint8_t last_receive_byte = 0x7F;
    static uint8_t u1_flag = 0;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t received_byte = USART_ReceiveData(USART1);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);

        if (u1_flag == 0) {
            if (received_byte == 0x5A && last_receive_byte == 0x7F) {
                u1_flag = 1;
                u1_index = 0;
            } else {
                last_receive_byte = received_byte;
                return;
            }
        }
        u1_rx_buffer[u1_index++] = received_byte;

        if(u1_index >= U1_RX_LEN) {
            u1_index = 0;
            if(u1_rx_buffer[0] == 0x5A && u1_rx_buffer[U1_RX_LEN-1] == 0x7F) {
                //todo: 传递消息。
                // memcpy(ctrl_msgs, &u1_rx_buffer[3], 8);
                ctrl_msg_type = u1_rx_buffer[1];
                ctrl_id = u1_rx_buffer[2];

                ctrl_msgs[0] = u1_rx_buffer[3];
                ctrl_msgs[1] = u1_rx_buffer[4];
                ctrl_msgs[2] = u1_rx_buffer[5];
                ctrl_msgs[3] = u1_rx_buffer[6];
                ctrl_msgs[4] = u1_rx_buffer[7];
                ctrl_msgs[5] = u1_rx_buffer[8];
                ctrl_msgs[6] = u1_rx_buffer[9];
                ctrl_msgs[7] = u1_rx_buffer[10];
                // debug_printf("rec: %d %d\r\n", ctrl_msg_type, ctrl_id);
            }else {
                u1_flag = 0;
            }
        }
        last_receive_byte = received_byte;
    }
}

static uint8_t is_initialized = 0;

int main(void) {
    // Initialization
    InitPeripherals();
    ADC_InitConfig();
    // Adc_Init();

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