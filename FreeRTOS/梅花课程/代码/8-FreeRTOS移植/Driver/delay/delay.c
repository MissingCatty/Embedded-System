#include "delay.h"

void _timer_base_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_TimeBaseStructInit(&TIM_InitStructure);

    TIM_InitStructure.TIM_Prescaler     = 84 - 1;             // CNT的时钟频率 = 42M / (42 - 1) = 1000000 Hz
    TIM_InitStructure.TIM_CounterMode   = TIM_CounterMode_Up; // 向上计数
    TIM_InitStructure.TIM_Period        = 0xFFFFFFFF;         // 配置为最大计数值，则无需考虑溢出
    TIM_InitStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM2, &TIM_InitStructure);

    TIM_Cmd(TIM2, ENABLE);
}

void delay_init(void)
{
    _timer_base_init();
}

void delay_us(uint32_t us)
{
    uint32_t tickstart = TIM2->CNT;
    while ((TIM2->CNT - tickstart) < us);
}

void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        delay_us(1000);
    }
}

void delay_s(uint32_t s)
{
    while (s--)
    {
        delay_ms(1000);
    }
}
