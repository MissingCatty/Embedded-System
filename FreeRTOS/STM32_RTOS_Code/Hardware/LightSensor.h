#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include "Experiment_Selected.h"

#define LightSensor_PORT    GPIOB
#define LightSensor_PORT_MODE   GPIO_Mode_IN_FLOATING
#define LightSensor_PORT_PIN    GPIO_Pin_7

#ifdef EXP_GPIO_LIGHTSENSOR
    #define LightSensor_PORT    GPIOB
    #define LightSensor_PORT_MODE   GPIO_Mode_IN_FLOATING
    #define LightSensor_PORT_PIN    GPIO_Pin_7
#endif

void LightSensor_Init(void);

#endif
