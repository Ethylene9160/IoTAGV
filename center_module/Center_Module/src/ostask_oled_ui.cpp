#include "ostask_oled_ui.h"

#include "stdio.h"

#include "vehicle_controller.h"

#include "user_gpio.h"
#include "user_i2c.h"

#include "u8g2.h"

#include "hmc5883l.h"
#include "usart.h"

namespace ostask_oled_ui {

    UserGPIO KeyConfirm("A0", false), Key0("C13", true), Key1("C1", true);
    UserI2CMaster Oled("B6", "B7");
    u8g2_t u8g2;

    enum Page { MAIN_PAGE, LIST_PAGE, SETTING_PAGE };
    Page currentPage = MAIN_PAGE;

    const char* listItems[] = { "Set target X", "Set target Y", "Back" }; //, "None", "None", "None" };
    int listItemsCount = sizeof(listItems) / sizeof(listItems[0]);
    int currentItem = 0;
    float settingConfigs[2][3] = {
        { -1.5f, 3.5f, 0.1f },
        { 0.0f, 5.0f, 0.1f }
    };
    float settingValue = 0.0f;

    vehicle_controller* controller = nullptr;

    void initGPIO() {
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
        osDelay(200);
    }

    void drawMainPage() {
        u8g2_ClearBuffer(&u8g2);

        char buffer[30];
        snprintf(buffer, sizeof(buffer), "Current: (%.1f, %.1f)", controller->get_self_point().x, controller->get_self_point().y);
        u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
        u8g2_DrawStr(&u8g2, 0, 24, buffer);
        snprintf(buffer, sizeof(buffer), "Target: (%.1f, %.1f)", controller->get_target_point().x, controller->get_target_point().y);
        u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
        u8g2_DrawStr(&u8g2, 0, 48, buffer);

        u8g2_SendBuffer(&u8g2);
    }

    void drawListPage() {
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
        for (int i = 0; i < listItemsCount; ++i) {
            if (i == currentItem) {
                u8g2_DrawBox(&u8g2, 0, i * 10, 128, 10);
                u8g2_SetDrawColor(&u8g2, 0);
            }
            u8g2_DrawStr(&u8g2, 2, (i + 1) * 10, listItems[i]);
            if (i == currentItem) {
                u8g2_SetDrawColor(&u8g2, 1);
            }
        }
        u8g2_SendBuffer(&u8g2);
    }

    void drawSettingPage() {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "Value: %.1f", settingValue);

        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
        u8g2_DrawStr(&u8g2, 0, 24, buffer);
        u8g2_SendBuffer(&u8g2);
    }

    void updateMainPage() {
        if (KeyConfirm.pressed()) {
            currentPage = LIST_PAGE;
        }
        drawMainPage();
    }

    void updateListPage() {
        if (KeyConfirm.pressed()) {
            if (currentItem == 0) {
                settingValue = controller->get_target_point().x;
                currentPage = SETTING_PAGE;
            } else if (currentItem == 1) {
                settingValue = controller->get_target_point().y;
                currentPage = SETTING_PAGE;
            } else if (currentItem == 2) {
                currentPage = MAIN_PAGE;
            } else {
                // TODO
            }
        } else if (Key0.pressed()) {
            currentItem = (currentItem + listItemsCount - 1) % listItemsCount;
        } else if (Key1.pressed()) {
            currentItem = (currentItem + 1) % listItemsCount;
        }
        drawListPage();
    }

    void updateSettingPage() {
        float settingMin = settingConfigs[currentItem][0];
        float settingMax = settingConfigs[currentItem][1];
        float settingStep = settingConfigs[currentItem][2];
        if (KeyConfirm.pressed()) {
            if (currentItem == 0) {
                controller->set_target_point({ settingValue, controller->get_target_point().y });
            } else if (currentItem == 1) {
                controller->set_target_point({ controller->get_target_point().x, settingValue });
            }
            currentPage = LIST_PAGE;
        } else if (Key0.pressed(1)) {
            settingValue -= settingStep;
            if (settingValue < settingMin) settingValue = settingMin;
        } else if (Key1.pressed(1)) {
            settingValue += settingStep;
            if (settingValue > settingMax) settingValue = settingMax;
        }
        drawSettingPage();
    }

    void taskProcedure(void *argument) {
        controller = static_cast<vehicle_controller *>(argument);

        // initialization
        initGPIO();
        initU8g2Platform();

        /*
         * TODO Test Magnetic Sensor Begin
         */
        hmc5883l_Init();
        osDelay(100);
        float init_alpha = 0.0f;
        for(int i = 0; i < 100; ++i) {
            init_alpha += hmc5883l_GetAngle()/100.0f;
            osDelay(10);
        }
        controller->set_init_alpha(init_alpha);
        /*
         * TODO Test Magnetic Sensor End
         */

        while (true) {
            switch (currentPage) {
                case MAIN_PAGE:
                    updateMainPage();
                    break;
                case LIST_PAGE:
                    updateListPage();
                    break;
                case SETTING_PAGE:
                    updateSettingPage();
                    break;
            }
            osDelay(6);

            /*
             * TODO Test Magnetic Sensor Begin
             */
            float res = hmc5883l_GetAngle();
            controller -> set_current_alpha(res);
            // char buffer[20];
            // snprintf(buffer, sizeof(buffer), "%6.2f\n", res);
            // HAL_UART_Transmit(&huart2, (uint8_t*) buffer, 7, HAL_MAX_DELAY);
            /*
             * TODO Test Magnetic Sensor End
             */
        }
    }

} // namespace ostask_oled_ui