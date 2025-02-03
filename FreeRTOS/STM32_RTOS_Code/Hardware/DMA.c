#include "DMA.h"

#ifdef EXP_ADC_DMA

uint16_t ADC_CH123_Data[3];                         // adc通道123数据存储地址
#define ADC_DR_Address (uint32_t)(&ADC1->DR);       // ADC规则数据寄存器地址
#define ADC_Res_Address (uint32_t)(ADC_CH123_Data); // SRAM中数组地址

void DMA_Config(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // 连续模式，计数器到0后自动重装，并且重新装载源地址和目的地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                          // 传输方向：外设to内存
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC_DR_Address;                  // 外设寄存器地址
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 外设数据宽度，16位
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // 外设地址不自增
    DMA_InitStructure.DMA_MemoryBaseAddr = ADC_Res_Address;                     // 内存地址
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;     // 内存数据宽度，16位
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // 内存地址自增
    DMA_InitStructure.DMA_BufferSize = 3;                                       // 传输3次
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                // 硬件触发
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel1, ENABLE);
}

uint16_t *DMA_GetResAddr(void)
{
    return ADC_CH123_Data;
}
#endif
