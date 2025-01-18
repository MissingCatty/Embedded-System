#ifndef TIMER_H
#define TIMER_H

#include "stm32f10x.h"
#include "Experiment_Selected.h"

/*
** 公共函数
*/
void Timer_Init(void);

/*
** 定制函数
*/
#ifdef EXP_TIM_IT_UPDATE
    uint8_t Timer_GetSeconds(void);
#endif

#ifdef EXP_TIM_IC
    uint16_t TIM_GetFrequency(void);
    float TIM_GetDutyRatio(void);
#endif

#endif
