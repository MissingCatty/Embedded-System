#include "LED.h"

void LED_Config(uint16_t LEDx)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = LEDx;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 用于配置输入模式或开漏模式下的上拉/下拉状态
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
}

void LED_ON(uint16_t LEDx)
{
    GPIO_WriteBit(LED_PORT, LEDx, Bit_RESET);
}

void LED_OFF(uint16_t LEDx)
{
    GPIO_WriteBit(LED_PORT, LEDx, Bit_SET);
}
