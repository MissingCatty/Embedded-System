#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "Experiment_Selected.h"

void Exp_GPIO_LightSensor(void);
void Exp_TIM_IT_Update(void);
void Exp_TIM_PWM(void);
void Exp_TIM_IC(void);
void Exp_ADC_DMA(void);
void Exp_USART(void);
void Exp_I2C(void);
void Exp_SPI(void);

#endif // !__EXPERIMENT_H
