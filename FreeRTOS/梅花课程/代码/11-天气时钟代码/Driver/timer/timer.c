#include "timer.h"
#include "FreeRTOS.h"

volatile static uint64_t ticks_us = 0; // us数

void _timer_base_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    RCC_ClocksTypeDef RCC_ClocksStructure;
    RCC_GetClocksFreq(&RCC_ClocksStructure);
    // 供给APB1上的定时器的时钟（TIMxCLK）将是 PCLK1 的两倍
    uint32_t apb1_tim_freq_mhz = RCC_ClocksStructure.PCLK1_Frequency / 1000 / 1000 * 2;

    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_TimeBaseStructInit(&TIM_InitStructure);
    TIM_InitStructure.TIM_Prescaler     = apb1_tim_freq_mhz - 1; // CNT的时钟频率 = 42M / (42 - 1) = 1000000 Hz
    TIM_InitStructure.TIM_CounterMode   = TIM_CounterMode_Up;    // 向上计数
    TIM_InitStructure.TIM_Period        = 999;                   // 配置为最大计数值，则无需考虑溢出
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_InitStructure);

    TIM_Cmd(TIM2, ENABLE);
}

void _timer_it_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void timer_init(void)
{
    _timer_base_init();
    _timer_it_init();
}

uint64_t timer_now_us(void)
{
    uint64_t now;

    now = ticks_us + TIM_GetCounter(TIM2);

    return now;
}

uint64_t timer_now_ms(void)
{
    return timer_now_us() / 1000;
}

uint64_t timer_now_s(void)
{
    return timer_now_us() / 1000000;
}

void timer_delay_us(uint32_t us)
{
    uint64_t now = timer_now_us();
    while (timer_now_us() - (uint32_t)now < us);
}

void timer_delay_ms(uint32_t ms)
{
    timer_delay_us(ms * 1000);
}

void timer_delay_s(uint32_t s)
{
    timer_delay_us(s * 1000000);
}

void TIM2_IRQHandler(void)
{
    // 1ms产生一个中断
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        ticks_us += 1000;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
