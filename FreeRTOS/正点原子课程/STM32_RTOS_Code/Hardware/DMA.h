#ifndef DMA_H
#define DMA_H

#include "stm32f10x.h"
#include "Experiment_Selected.h"

void DMA_Config(void);

#ifdef EXP_ADC_DMA
    uint16_t* DMA_GetResAddr(void);
#endif

#endif
