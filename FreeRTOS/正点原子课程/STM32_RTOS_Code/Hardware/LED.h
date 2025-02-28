#ifndef LED_H
#define LED_H

#include "Experiment_Selected.h"

#define LED_PORT GPIOB
#define LED_PORT_MODE GPIO_Mode_Out_PP
#define LED_PORT_PIN GPIO_Pin_12
#define LED_PORT_SPEED GPIO_Speed_50MHz

#ifdef EXP_GPIO_LIGHTSENSOR
    #define LED_PORT GPIOB
    #define LED_PORT_MODE GPIO_Mode_Out_PP
    #define LED_PORT_PIN GPIO_Pin_12
    #define LED_PORT_SPEED GPIO_Speed_50MHz
#endif

void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);

#endif
