#include "lcd_oled.h"
#include "stdlib.h"
#include "lcd_oledfont.h"
#include "deca_sleep.h"
//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127



///I2C related
/*******************************************************************************
* Function Name  : I2C_delay
* Description    : �ӳ�ʱ��
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
static void I2C_delay(uint16_t cnt)
{
    while(cnt--);
}

#define IIC_TIMEOUT 100
int IIc_flag = 1 ;//����1�� ������0
/*******************************************************************************
* Function Name  : I2C_AcknowledgePolling
* Description    : �ȴ���ȡI2C���߿���Ȩ �ж�æ״̬
* Input          : - I2Cx:I2C�Ĵ�����ַ
*                  - I2C_Addr:��������ַ
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
static void I2C_AcknowledgePolling(I2C_TypeDef *I2Cx,uint8_t I2C_Addr)
{
    vu16 SR1_Tmp;
    uint16_t timeout = 10000;
    timeout = IIC_TIMEOUT;
    do
    {
        I2C_GenerateSTART(I2Cx, ENABLE); /*��ʼλ*/
        /*��SR1*/
        SR1_Tmp = I2C_ReadRegister(I2Cx, I2C_Register_SR1);
        /*������ַ(д)*/
        I2C_Send7bitAddress(I2Cx, I2C_Addr, I2C_Direction_Transmitter);

    }
    while(!(I2C_ReadRegister(I2Cx, I2C_Register_SR1) & 0x0002)&&(timeout--));
    I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
    I2C_GenerateSTOP(I2Cx, ENABLE);  /*ֹͣλ*/
}


/*******************************************************************************
* Function Name  : I2C_WriteOneByte
* Description    : ͨ��ָ��I2C�ӿ�д��һ���ֽ�����
* Input          : - I2Cx:I2C�Ĵ�����ַ
*                  - I2C_Addr:��������ַ
*                  - addr:Ԥд���ֽڵ�ַ
*                  - value:д������
* Output         : None
* Return         : �ɹ�����0
* Attention      : None
*******************************************************************************/
uint8_t I2C_WriteOneByte(I2C_TypeDef *I2Cx,uint8_t I2C_Addr,uint8_t addr,uint8_t value)
{
    uint16_t timeout = 0;
    /* ��ʼλ */
    I2C_GenerateSTART(I2Cx, ENABLE);
    timeout = IIC_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) &&(timeout--));
    I2C_Send7bitAddress(I2Cx, I2C_Addr, I2C_Direction_Transmitter);

    if(timeout == 0)
        IIc_flag = 0;

    timeout = IIC_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)&&(timeout--));
    /*���͵�ַ*/
    I2C_SendData(I2Cx, addr);
    timeout = IIC_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)&&(timeout--));
    /* дһ���ֽ�*/
    I2C_SendData(I2Cx, value);
    timeout = IIC_TIMEOUT;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)&&(timeout--));
    /* ֹͣλ*/
    I2C_GenerateSTOP(I2Cx, ENABLE);
    /*stop bit flag*/
    //while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF));

    I2C_AcknowledgePolling(I2Cx,I2C_Addr);
    I2C_delay(80);
    return 0;
}


uint8_t I2C_ReadOneByte(I2C_TypeDef* I2Cx, unsigned char AddressDevice, unsigned char AddressByte)
{
    uint8_t ReceiveData = 0;

    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2Cx, ENABLE);
    while( !I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) ); // wait Generate Start

    I2C_Send7bitAddress(I2Cx, AddressDevice, I2C_Direction_Transmitter);
    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) ); // wait send Address Device

    I2C_SendData(I2Cx, AddressByte);
    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED) ); // wait send Address Byte

    I2C_GenerateSTART(I2Cx, ENABLE);
    while( !I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) ); // wait Generate Start

    I2C_Send7bitAddress(I2Cx, AddressDevice, I2C_Direction_Receiver);
    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) ); // wait Send Address Device As Receiver

    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) ); // wait Receive a Byte
    ReceiveData = I2C_ReceiveData(I2Cx);

    I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current); // send not acknowledge
    I2C_AcknowledgeConfig(I2Cx, DISABLE);// disable acknowledge

    I2C_GenerateSTOP(I2Cx, ENABLE);
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF)); // wait Generate Stop Condition

    I2C_AcknowledgeConfig(I2Cx, DISABLE);// disable acknowledge

    return ReceiveData;
};


/*******************************************************************************
* Function Name  : I2C_Write
* Description    : ͨ��ָ��I2C�ӿ�д�����ֽ�����
* Input          : - I2Cx:I2C�Ĵ�����ַ
*                  - I2C_Addr:��������ַ
*                  - addr:Ԥд���ֽڵ�ַ
*                  - *buf:Ԥд�����ݴ洢λ��
*                  - num:д���ֽ���
* Output         : None
* Return         : �ɹ�����0
* Attention      : None
*******************************************************************************/
uint8_t I2C_Write(I2C_TypeDef *I2Cx,uint8_t I2C_Addr,uint8_t addr,uint8_t *buf,uint16_t num)
{
    uint8_t err=0;

    while(num--)
    {
        if(I2C_WriteOneByte(I2Cx, I2C_Addr,addr++,*buf++))
        {
            err++;
        }
    }
    if(err)
        return 1;
    else
        return 0;
}


//STM32F103C8T6 IIC OLED
//#ifdef STM32F10X_MD
#if OLED_MODE==1
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
    if(IIc_flag == 1)
    {
        if(cmd)
        {
            I2C_WriteOneByte(I2C1,0x7e,0x40,dat);
            // I2C_WriteOneByte(I2C1,0x78,0x40,dat);
        }
        else
        {
            I2C_WriteOneByte(I2C1,0x7e,0x00,dat);
            //I2C_WriteOneByte(I2C1,0x78,0x00,dat);
        }
    }
}
#else
//No use at all
/*void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
    DATAOUT(dat);
    if(cmd)
        OLED_DC_Set();
    else
        OLED_DC_Clr();
    OLED_CS_Clr();
    OLED_WR_Clr();
    OLED_WR_Set();
    OLED_CS_Set();
    OLED_DC_Set();
}
#else*/
//��SSD1106д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
uint8_t i;
if(cmd)
OLED_DC_Set();
else
OLED_DC_Clr();
OLED_CS_Clr();
for(i=0; i<8; i++)
{
OLED_SCLK_Clr();
if(dat&0x80)
OLED_SDIN_Set();
else
OLED_SDIN_Clr();
OLED_SCLK_Set();
dat<<=1;
}
OLED_CS_Set();
OLED_DC_Set();
}
#endif
void OLED_Set_Pos(unsigned char x, unsigned char y)
{
    OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD);
}
//����OLED��ʾ
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//�ر�OLED��ʾ
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}
//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!
void OLED_Clear(void)
{
    uint8_t i,n;
    for(i=0; i<8; i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7��
        OLED_WR_Byte (0x00,OLED_CMD);      //������ʾλ�á��е͵�ַ
        OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ
        for(n=0; n<135; n++)OLED_WR_Byte(0,OLED_DATA);
    } //������ʾ
}

void LCD_ClearLines(void)
{
    uint8_t i,n;
    for(i=2; i<8; i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7��
        OLED_WR_Byte (0x00,OLED_CMD);      //������ʾλ�á��е͵�ַ
        OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ
        for(n=0; n<135; n++)OLED_WR_Byte(0,OLED_DATA);
    } //������ʾ
}


//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ
//size:ѡ������ 16/12
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr)
{
    unsigned char c=0,i=0;
    c=chr-' ';//�õ�ƫ�ƺ��ֵ
    if(x>Max_Column-1)
    {
        x=0;
        y=y+2;
    }
    x = x+2; //add offset
    if(SIZE ==16)
    {
        OLED_Set_Pos(x,y);
        for(i=0; i<8; i++)
            OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
        OLED_Set_Pos(x,y+1);
        for(i=0; i<8; i++)
            OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
    }
    else
    {
        OLED_Set_Pos(x,y+1);
        for(i=0; i<6; i++)
            OLED_WR_Byte(F6x8[c][i],OLED_DATA);
    }
}
//m^n����
uint32_t oled_pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)result*=m;
    return result;
}
//��ʾ2������
//x,y :�������
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ 0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{
    uint8_t t,temp;
    uint8_t enshow=0;
    for(t=0; t<len; t++)
    {
        temp=(num/oled_pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                OLED_ShowChar(x+(size/2)*t,y,' ');
                continue;
            }
            else enshow=1;

        }
        OLED_ShowChar(x+(size/2)*t,y,temp+'0');
    }
}
//��ʾһ���ַ��Ŵ�
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr)
{
    unsigned char j=0;
    while (chr[j]!='\0')
    {
        OLED_ShowChar(x,y,chr[j]);
        x+=8;
        if(x>120)
        {
            x=0;
            y+=2;
        }
        j++;
    }
}
//��ʾ����
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no)
{
    uint8_t t,adder=0;
    OLED_Set_Pos(x,y);
    for(t=0; t<16; t++)
    {
        OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
        adder+=1;
    }
    OLED_Set_Pos(x,y+1);
    for(t=0; t<16; t++)
    {
        OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
        adder+=1;
    }
}
/***********������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{
    unsigned int j=0;
    unsigned char x,y;

    if(y1%8==0) y=y1/8;
    else y=y1/8+1;
    for(y=y0; y<y1; y++)
    {
        OLED_Set_Pos(x0,y);
        for(x=x0; x<x1; x++)
        {
            OLED_WR_Byte(BMP[j++],OLED_DATA);
        }
    }
}

//                      1    2    3    4    5    6    7    8    9    10   11   12   13
//unsigned char InitCmd[]={0xE2,0xA1,0xC8,0x24,0x81,0x23,0x2C,0x2E,0x2F,0xA4,0xA6,0xAF};
unsigned char InitCmd[]= {0xAE,0x00,0x10,0x40,0x81,0xCF,0xA1,0xC8,0xA6,0xA8,0x3F,0xD3,0x00,0xD5,0x80,0xD9,0xF1,0xDA,0x12,0xDB,0x40,0x20,0x02,0x8D,0x14,0xA4,0xA6};

//��ʼ��SSD1306
void OLED_Init(void)
{

    //STM32F103RC SPI OLED
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    //ʹ��A�˿�ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);    //��ʼ��GPIOD3,6
    GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15);

    OLED_RST_Set();
    deca_sleep(100);
    OLED_RST_Clr();
    deca_sleep(200);
    OLED_RST_Set();
    for(unsigned char i=0; i<sizeof(InitCmd); i++)
    {
        OLED_WR_Byte( InitCmd[i],OLED_CMD);
    }
    OLED_Clear();
    OLED_WR_Byte(0xAF,OLED_CMD);
}
