#include "stm32f10x.h"
#include "i2c.h"
unsigned char SlaveAddress = 0xA0;

typedef enum {FALSE = 0, TRUE = !FALSE} bool;

/*��־�Ƿ��������*/
char  test=0;
/*I2C���豸*/
/*ģ��IIC�˿�������붨��*/
#define SCL_H         GPIOB->BSRR = GPIO_Pin_6
#define SCL_L         GPIOB->BRR  = GPIO_Pin_6 
#define SDA_H         GPIOB->BSRR = GPIO_Pin_7
#define SDA_L         GPIOB->BRR  = GPIO_Pin_7
#define SCL_read      GPIOB->IDR  & GPIO_Pin_6
#define SDA_read      GPIOB->IDR  & GPIO_Pin_7

/*I2C�Ķ˿ڳ�ʼ��---------------------------------------*/
void I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

/*I2C����ʱ����-----------------------------------------*/
void I2C_delay(void)
{
	u8 i=30; //��������Ż��ٶ�	����������͵�5����д��
	while(i) 
	{ 
		i--; 
	}  
}

/*I2C�ĵȴ�5ms����--------------------------------------*/
void delay5ms(void)
{
	int i=5000;  
	while(i) 
	{ 
		i--; 
	}  
}

/*I2C��������-------------------------------------------*/
bool I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)return FALSE;	//SDA��Ϊ�͵�ƽ������æ,�˳�
	SDA_L;
	I2C_delay();
	if(SDA_read) return FALSE;	//SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
	SDA_L;
	I2C_delay();
	return TRUE;
}

/*I2Cֹͣ����-------------------------------------------*/
void I2C_Stop(void)
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
} 

/*I2C��ACK����------------------------------------------*/
void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}   

/*I2C��NoACK����----------------------------------------*/
void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
} 

/*I2C�ȴ�ACK����----------------------------------------*/
bool I2C_WaitAck(void) 	 //����Ϊ:=1��ACK,=0��ACK
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
		SCL_L;
		I2C_delay();
		return FALSE;
	}
	SCL_L;
	I2C_delay();
	return TRUE;
}

/*I2C����һ��u8���ݺ���---------------------------------*/
void I2C_SendByte(u8 SendByte) //���ݴӸ�λ����λ//
{
    u8 i=8;
    while(i--)
    {
        SCL_L;
        I2C_delay();
		if(SendByte&0x80)
			SDA_H;  
		else 
			SDA_L;   
        SendByte<<=1;
        I2C_delay();
		SCL_H;
        I2C_delay();
    }
    SCL_L;
}  

/*I2C��ȡһ��u8���ݺ���---------------------------------*/
unsigned char I2C_RadeByte(void)  //���ݴӸ�λ����λ//
{ 
    u8 i=8;
    u8 ReceiveByte=0;
	
    SDA_H;				
    while(i--)
    {
		ReceiveByte<<=1;      
		SCL_L;
		I2C_delay();
		SCL_H;
		I2C_delay();	
		if(SDA_read)
		{
			ReceiveByte|=0x01;
		}
    }
    SCL_L;
    return ReceiveByte;
}  

/*I2C��ָ���豸ָ����ַд��u8����-----------------------*/
void Single_WriteI2C(unsigned int REG_Address,unsigned char REG_data)//���ֽ�д��
{
	if(!I2C_Start())return;
    I2C_SendByte(SlaveAddress);   //�����豸��ַ+д�ź�//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//���ø���ʼ��ַ+������ַ 
    if(!I2C_WaitAck()){I2C_Stop(); return;}
			//AT24C02 remove high address
//    I2C_SendByte((unsigned char)(REG_Address&0xFF00)>>8);   	//����high address	  
//    I2C_WaitAck();	
	  I2C_SendByte((unsigned char)(REG_Address&0xFF) );   //���õ���ʼ��ַ      
    I2C_WaitAck();		
    I2C_SendByte(REG_data);
    I2C_WaitAck();   
    I2C_Stop(); 
    delay5ms(); delay5ms(); delay5ms(); delay5ms(); delay5ms();
}

/*I2C��ָ���豸ָ����ַ����u8����-----------------------*/
unsigned char Single_ReadI2C(unsigned int REG_Address)//��ȡ���ֽ�
{   
	unsigned char REG_data;  
	if(!I2C_Start())return 0;
    I2C_SendByte(SlaveAddress);   //�����豸��ַ+д�ź�//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//���ø���ʼ��ַ+������ַ 
    if(!I2C_WaitAck()){I2C_Stop(); return 0;}
		//AT24C02 remove high address
//	I2C_SendByte((unsigned char)(REG_Address&0xFF00)>>8);	//����high address	  
//	I2C_WaitAck();  	
	I2C_SendByte((unsigned char)(REG_Address&0xFF) );   //���õ���ʼ��ַ		
	I2C_WaitAck();  
	I2C_Start();	
	I2C_SendByte(SlaveAddress+1);
	I2C_WaitAck();  	
	REG_data= I2C_RadeByte();
	I2C_NoAck(); 	
	I2C_Stop(); 	
	//return TRUE;
	return REG_data;
}
