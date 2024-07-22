#include "adc.h"

#include "stm32f10x.h"

inline void ADC_InitConfig(void) {
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    // 使能ADC1时钟和GPIOA时钟（假设我们使用ADC1和PA0）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置PA0为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 复位并配置ADC
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // 配置ADC1通道0
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);

    // 启用ADC1
    ADC_Cmd(ADC1, ENABLE);

    // 启用ADC1复位校准寄存器
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));

    // 启用ADC1校准
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    // 启动ADC1软件转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

inline uint16_t ADC_Read(void) {
    // 等待转换完成
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    // 读取ADC值
    return ADC_GetConversionValue(ADC1);
}

inline uint32_t ADCRandomNumber(uint32_t low, uint32_t high) {
    uint32_t random_number = 0;
    for (int i = 0; i < 32; i += 12) {
        random_number |= (ADC_Read() & 0xFFF) << i; // 使用ADC的低12位
    }
    // 将生成的随机数缩放到指定范围内
    return low + random_number % (high - low + 1);
}


//
//
// void  Adc_Init(void)
// {
//     ADC_InitTypeDef ADC_InitStructure;
//     GPIO_InitTypeDef GPIO_InitStructure;
//
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1, ENABLE );      //使能ADC1通道时钟
//
//
//     RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
//
//     //PA1 作为模拟通道输入引脚, 一定要用悬空脚,否则获取的随机数,不随机
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;        //模拟输入引脚
//     GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//     ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值
//
//     ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;    //ADC工作模式:ADC1和ADC2工作在独立模式
//     ADC_InitStructure.ADC_ScanConvMode = DISABLE;    //模数转换工作在单通道模式
//     ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//DISABLE;    //模数转换工作在单次转换模式
//     ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;    //转换由软件而不是外部触发启动
//     ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;    //ADC数据右对齐
//     ADC_InitStructure.ADC_NbrOfChannel = 1;    //顺序进行规则转换的ADC通道的数目
//     ADC_Init(ADC1, &ADC_InitStructure);    //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
//
//     ADC_Cmd(ADC1, ENABLE);    //使能指定的ADC1
//     ADC_ResetCalibration(ADC1);    //使能复位校准
//     while(ADC_GetResetCalibrationStatus(ADC1));    //等待复位校准结束
//     ADC_StartCalibration(ADC1);     //开启AD校准
//     while(ADC_GetCalibrationStatus(ADC1));     //等待校准结束
//     ADC_SoftwareStartConvCmd(ADC1, ENABLE);        //使能指定的ADC1的软件转换启动功能
// }
//
// //获得ADC值
// //ch:通道值 0~3
// /*采集4次ADC的值，每次取采集的第四位，拼成16位作为种子*/
// uint32_t Get_Adc_RandomSeek(uint32_t high, uint32_t low)
// {
//     uint8_t Count;
//     uint32_t ADC_RandomSeek = 0;
//
//     //设置指定ADC的规则组通道，一个序列，采样时间
//     ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5 );    //ADC1,ADC通道,采样时间为239.5周期
//     ADC_SoftwareStartConvCmd(ADC1, ENABLE);        //使能指定的ADC1的软件转换启动功能
//     for(Count = 0; Count < 4; Count++){
//         while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
//         ADC_RandomSeek <<= 4;
//         ADC_RandomSeek += ADC_GetConversionValue(ADC1) & 0x000f;		/*采集4次ADC的值，每次取采集的第四位，拼成16位作为种子*/
//     }
//     ADC_SoftwareStartConvCmd(ADC1,DISABLE);
//
//     ADC_RandomSeek = ADC_RandomSeek % (high - low + 1) + low;		/*将生成的随机数缩放到指定范围内*/
//     return ADC_RandomSeek;
// }
