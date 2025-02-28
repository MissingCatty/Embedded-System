#include "LightSensor.h"
#include "stm32f10x.h"

void LightSensor_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = LightSensor_PORT_MODE;
    GPIO_InitStructure.GPIO_Pin = LightSensor_PORT_PIN;
    GPIO_Init(LightSensor_PORT, &GPIO_InitStructure);
}
