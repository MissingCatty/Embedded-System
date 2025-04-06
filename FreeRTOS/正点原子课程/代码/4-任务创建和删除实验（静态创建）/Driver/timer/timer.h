#ifndef TIMER_H
#define TIMER_H

#include "stm32f4xx.h"

typedef struct
{
    uint32_t     RCC_APB1Periph_TIMx;
    TIM_TypeDef *TIMx;
    uint16_t     TIM_Prescaler;
    uint16_t     TIM_CounterMode;
    uint32_t     TIM_Period;
    uint16_t     TIM_ClockDivision;
    uint16_t     TIM_RepetitionCounter;
} timer_base_conf_t;

void timer_init(timer_base_conf_t *timer_base_conf);
void timer_cmd(timer_base_conf_t *timer_base_conf, FunctionalState NewState);

#endif
