#ifndef ADC_H
#define ADC_H

#include "stm32f10x.h"
#include "Experiment_Selected.h"

void ADC_Config(void);
uint16_t ADC_ReadValue(void);

#endif // !ADC_H
