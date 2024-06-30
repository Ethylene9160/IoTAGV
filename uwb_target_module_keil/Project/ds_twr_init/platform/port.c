/*! ----------------------------------------------------------------------------
 * @file    port.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */
#include "deca_sleep.h"
#include "port.h"
#include "lcd_oled.h"

#define systick_init(x)             SysTick_Configuration(x)
#define spi_init(x)                 SPI_Configuration(x)
#define gpio_init(x)                GPIO_Configuration(x)

/* System tick 32 bit variable defined by the platform */
extern __IO unsigned long time32_incr;

/* Internal functions prototypes. */
//static void LCD_Configuration(void);
static void spi_peripheral_init(void);

int No_Configuration(void)
{
    return -1;
}

unsigned long portGetTickCnt(void)
{
    return time32_incr;
}

int SysTick_Configuration(void)
{
    if (SysTick_Config(SystemCoreClock / CLOCKS_PER_SEC))
    {
        /* Capture error */
        while (1);
    }
    NVIC_SetPriority (SysTick_IRQn, 5);

    return 0;
}

void SPI_ChangeRate(uint16_t scalingfactor)
{
    uint16_t tmpreg = 0;
    /* Get the SPIx CR1 value */
    tmpreg = SPIx->CR1;
    /*clear the scaling bits*/
    tmpreg &= 0xFFC7;
    /*set the scaling bits*/
    tmpreg |= scalingfactor;
    /* Write to SPIx CR1 */
    SPIx->CR1 = tmpreg;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_set_rate_low()
 *
 * @brief Set SPI rate to less than 3 MHz to properly perform DW1000 initialisation.
 *
 * @param none
 *
 * @return none
 */
void spi_set_rate_low (void)
{
    SPI_ChangeRate(SPI_BaudRatePrescaler_32);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_set_rate_high()
 *
 * @brief Set SPI rate as close to 20 MHz as possible for optimum performances.
 *
 * @param none
 *
 * @return none
 */
void spi_set_rate_high (void)
{
    SPI_ChangeRate(SPI_BaudRatePrescaler_4);
}

void SPI_ConfigFastRate(uint16_t scalingfactor)
{
    SPI_InitTypeDef SPI_InitStructure;

    SPI_I2S_DeInit(SPIx);

    // SPIx Mode setup
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;   //
    //SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    //SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //
    //SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = scalingfactor; //sets BR[2:0] bits - baudrate in SPI_CR1 reg bits 4-6
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPIx, &SPI_InitStructure);

    // Enable SPIx
    SPI_Cmd(SPIx, ENABLE);
}

int SPI_Configuration(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    SPI_I2S_DeInit(SPIx);

    // SPIx Mode setup
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;   //
    //SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    //SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //
    //SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; //sets BR[2:0] bits - baudrate in SPI_CR1 reg bits 4-6
    SPI_InitStructure.SPI_BaudRatePrescaler = SPIx_PRESCALER;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPIx, &SPI_InitStructure);

    // SPIx SCK and MOSI pin setup
    GPIO_InitStructure.GPIO_Pin = SPIx_SCK | SPIx_MOSI;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(SPIx_GPIO, &GPIO_InitStructure);

    // SPIx MISO pin setup
    GPIO_InitStructure.GPIO_Pin = SPIx_MISO;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;

    GPIO_Init(SPIx_GPIO, &GPIO_InitStructure);

    // SPIx CS pin setup
    GPIO_InitStructure.GPIO_Pin = SPIx_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(SPIx_CS_GPIO, &GPIO_InitStructure);

    // Disable SPIx SS Output
    SPI_SSOutputCmd(SPIx, DISABLE);

    // Enable SPIx
    SPI_Cmd(SPIx, ENABLE);

    // Set CS high
    GPIO_SetBits(SPIx_CS_GPIO, SPIx_CS);

    return 0;
}


int GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure all unused GPIO port pins in Analog Input mode (floating input
    * trigger OFF), this will reduce the power consumption and increase the device
    * immunity against EMI/EMC */

    // Enable GPIOs clocks
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
        ENABLE);

    // Set all GPIO pins as analog inputs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_1|GPIO_Pin_2;              //INX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        //ÍÆÍìÊä³ö
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //IO¿ÚËÙ¶ÈÎª50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);                  //¸ù¾ÝÉè¶¨²ÎÊý³õÊ¼»¯GPIOB.5

    return 0;
}


void reset_DW1000(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable GPIO used for DW1000 reset
    GPIO_InitStructure.GPIO_Pin = DW1000_RSTn;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);
    //drive the RSTn pin low
    GPIO_WriteBit(GPIOB, DW1000_RSTn, 0);
    deca_sleep(100);
    GPIO_WriteBit(GPIOB, DW1000_RSTn, 1);

    deca_sleep(100);
    //put the pin back to tri-state ... as input
    GPIO_InitStructure.GPIO_Pin = DW1000_RSTn;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);
}

void led_off (led_t led)
{
    switch (led)
    {
    case LED_PA1:
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        break;
    case LED_PA2:
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
        break;
    case LED_PA3:
        GPIO_ResetBits(GPIOA, GPIO_Pin_3);
        break;
    case LED_ALL:
        GPIO_ResetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
        break;
    default:
        // do nothing for undefined led number
        break;
    }
}

void led_on (led_t led)
{
    switch (led)
    {
    case LED_PA1:
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
        break;
    case LED_PA2:
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
        break;
    case LED_PA3:
        GPIO_SetBits(GPIOA, GPIO_Pin_3);
        break;
    case LED_ALL:
        GPIO_SetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
        break;
    default:
        // do nothing for undefined led number
        break;
    }
}

/**
  * @brief  Configures COM port.
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
  *   contains the configuration information for the specified USART peripheral.
  * @retval None
  */
#include "stm32_eval.h"
void usartinit(void)
{
    USART_InitTypeDef USART_InitStructure;
    //GPIO_InitTypeDef GPIO_InitStructure;

    /* USARTx configured as follow:
          - BaudRate = 115200 baud
          - Word Length = 8 Bits
          - One Stop Bit
          - No parity
          - Hardware flow control disabled (RTS and CTS signals)
          - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    STM_EVAL_COMInit(COM1, &USART_InitStructure);

    printf("/*****************************************************************/\n");
    printf("/********More Information Please Visit Our Website*************/\n");
    printf("/***********************bphero.com.cn**************************/\n");
    printf("/***********************FM Version V3.1**************************/\n");
    printf("/*****************************************************************/\n");
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
void USART_puts(uint8_t *s,uint8_t len)
{
    int i;
    for(i=0; i<len; i++)
    {
        putchar(s[i]);
    }
}
int is_IRQ_enabled(void)
{
    return ((   NVIC->ISER[((uint32_t)(DECAIRQ_EXTI_IRQn) >> 5)]
                & (uint32_t)0x01 << (DECAIRQ_EXTI_IRQn & (uint8_t)0x1F)  ) ? 1 : 0) ;
}

//==============================Adding USART sending functions=======================
#define TX_BUFFER_SIZE 256

uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile uint16_t tx_write_index = 0;
volatile uint16_t tx_read_index = 0;

void UART_SendBuffer(void);

void UART_Transmit(uint8_t *data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        tx_buffer[tx_write_index++] = data[i];
        if (tx_write_index >= TX_BUFFER_SIZE)
        {
            tx_write_index = 0;
        }
    }
    UART_SendBuffer();
}

void UART_SendBuffer(void)
{
    // æ£€æŸ¥ TXE æ ‡å¿—
    if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET)
    {
        if (tx_read_index != tx_write_index)
        {
            uint8_t data = tx_buffer[tx_read_index++];
            if (tx_read_index >= TX_BUFFER_SIZE)
            {
                tx_read_index = 0;
            }
            USART_SendData(USART1, data);
            USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        }
    }
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        if (tx_read_index != tx_write_index)
        {
            uint8_t data = tx_buffer[tx_read_index++];
            if (tx_read_index >= TX_BUFFER_SIZE)
            {
                tx_read_index = 0;
            }
            USART_SendData(USART1, data);
        }
        else
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
        USART_ClearITPendingBit(USART1, USART_IT_TXE);
    }
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_peripheral_init()
 *
 * @brief Initialise all SPI peripherals at once.
 *
 * @param none
 *
 * @return none
 */
static void spi_peripheral_init(void)
{
    spi_init();//for dwm1000
}

void LED_TEST(void)
{
    led_on(LED_ALL);
    deca_sleep(100);
    led_off(LED_ALL);
    deca_sleep(100);
    led_on(LED_ALL);
    deca_sleep(100);
    led_off(LED_ALL);
}
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn peripherals_init()
 *
 * @brief Initialise all peripherals.
 *
 * @param none
 *
 * @return none
 */
void peripherals_init (void)
{
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
        ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); /*JTAG-DP   SW-DP  Enable PB4 */
    /* Enable SPI1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    gpio_init();
    systick_init();
    spi_peripheral_init();
    OLED_Init();
    OLED_Clear()    ;

    usartinit();
    LED_TEST();
	
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}