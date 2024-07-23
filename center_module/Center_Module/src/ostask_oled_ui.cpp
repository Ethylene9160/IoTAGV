#include "ostask_oled_ui.h"

#include "vehicle_controller.h"

#include "user_gpio.h"
#include "user_i2c.h"

#include "u8g2.h"

namespace ostask_oled_ui {

    UserGPIO KeyConfirm("A0"), Key0("C1"), Key1("C13");
    UserI2CMaster Oled("B6", "B7");
    u8g2_t u8g2;

    void init_gpio() {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        /* I2C: SCL (PB6), SDA (PB7) */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* GPIO: KeyConfirm (PA0), Key0 (PC1), Key1 (PC13) */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }

    uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr) {
        switch (msg) {
            // Function which implements a delay, arg_int contains the amount of ms
            case U8X8_MSG_DELAY_MILLI:
                osDelay(arg_int);
                break;
                // Function which delays 10us
            case U8X8_MSG_DELAY_10MICRO:
                for (int i = 0; i < 1442; i++) {
                    __NOP();
                }
                break;
                // Function which delays 100ns
            case U8X8_MSG_DELAY_100NANO:
                for (int i = 0; i < 14; i++) {
                    __NOP();
                }
                break;
                // Function to define the logic level of the I2C_CLOCK line
            case U8X8_MSG_GPIO_I2C_CLOCK:
                if (arg_int) Oled.getScl()->set();
                else Oled.getScl()->reset();
                break;
                // Function to define the logic level of the I2C_SDA line
            case U8X8_MSG_GPIO_I2C_DATA:
                if (arg_int) Oled.getSda()->set();
                else Oled.getSda()->reset();
                break;
                // A message was received which is not implemented, return 0 to indicate an error
            default:
                return 0;
        }

        return 1; // command processed successfully.
    }

    void initU8g2Platform() {
        u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8g2_gpio_and_delay_stm32);
        u8g2_InitDisplay(&u8g2);
        u8g2_SetPowerSave(&u8g2, 0);
        u8g2_ClearBuffer(&u8g2);
        osDelay(1000);
    }

    void taskProcedure(void *argument) {
        auto* controller = static_cast<vehicle_controller *>(argument);

        // initialization
        init_gpio();
        initU8g2Platform();

//        u8g2_SetFontMode(&u8g2, 1);
//        u8g2_SetFontDirection(&u8g2, 0);
//        u8g2_SetFont(&u8g2, u8g2_font_inb24_mf);
//        u8g2_DrawStr(&u8g2, 0, 20, "U");
//
//        u8g2_SetFontDirection(&u8g2, 1);
//        u8g2_SetFont(&u8g2, u8g2_font_inb30_mn);
//        u8g2_DrawStr(&u8g2, 21,8,"8");
//
//        u8g2_SetFontDirection(&u8g2, 0);
//        u8g2_SetFont(&u8g2, u8g2_font_inb24_mf);
//        u8g2_DrawStr(&u8g2, 51,30,"g");
//        u8g2_DrawStr(&u8g2, 67,30,"\xb2");
//
//        u8g2_DrawHLine(&u8g2, 2, 35, 47);
//        u8g2_DrawHLine(&u8g2, 3, 36, 47);
//        u8g2_DrawVLine(&u8g2, 45, 32, 12);
//        u8g2_DrawVLine(&u8g2, 46, 33, 12);
//
//        u8g2_SetFont(&u8g2, u8g2_font_4x6_tr);
//        u8g2_DrawStr(&u8g2, 1,54,"github.com/olikraus/u8g2");
//
//        u8g2_SendBuffer(&u8g2);
//        osDelay(1000);

        while (true) {
            // TODO
//            cart_point target_point = {-0.3f, 1.5f};
//            controller->set_target_point(target_point);

            osDelay(20);
        }
    }

} // namespace ostask_oled_ui