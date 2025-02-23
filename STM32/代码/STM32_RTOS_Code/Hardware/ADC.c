#include "ADC.h"
#include "stm32f10x.h"

#ifdef EXP_ADC_DMA
void ADC_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72Mhz / 6 = 12Mhz < 14Mhz

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                       // 配置为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3; // PA1/2/3: ADC1_CH1/2/3
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                     // adc1和adc2独立工作
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                           // 单通道还是多通道
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                 // 数据对齐
    ADC_InitStructure.ADC_NbrOfChannel = 3;                                // 通道数量
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                    // 连续转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; // 选择外部触发源
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_ExternalTrigConvCmd(ADC1, ENABLE);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_55Cycles5);

    ADC_DMACmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE);

    // ADC校准（固定流程）
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;
}

uint16_t ADC_ReadValue(void)
{
    // 等待转换完成
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        ;

    // 读取 ADC 转换值
    return ADC_GetConversionValue(ADC1);
}

#endif
