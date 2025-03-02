#include <timer.h>

void _timer_base_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_TimeBaseStructInit(&TIM_InitStructure);

    TIM_InitStructure.TIM_Prescaler     = 84 - 1;
    TIM_InitStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Period        = 1000 - 1;
    TIM_InitStructure.TIM_ClockDivision = 0;
    TIM_Init(TIM2, &TIM_InitStructure);
}

void _timer_it_init(void)
{
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void timer_init(void)
{
    _timer_base_init();
    _timer_it_init();
}
