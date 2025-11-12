#include "timer.h"

void timer_init(timer_base_conf_t *timer_base_conf)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(timer_base_conf->RCC_APB1Periph_TIMx, ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler         = timer_base_conf->TIM_Prescaler;
    TIM_TimeBaseStructure.TIM_CounterMode       = timer_base_conf->TIM_CounterMode;
    TIM_TimeBaseStructure.TIM_Period            = timer_base_conf->TIM_Period;
    TIM_TimeBaseStructure.TIM_ClockDivision     = timer_base_conf->TIM_ClockDivision;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = timer_base_conf->TIM_RepetitionCounter;
    TIM_TimeBaseInit(timer_base_conf->TIMx, &TIM_TimeBaseStructure);
}

void timer_cmd(timer_base_conf_t *timer_base_conf, FunctionalState NewState)
{
    TIM_Cmd(timer_base_conf->TIMx, NewState);
}
