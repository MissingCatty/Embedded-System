#include "LED.h"
#include "stm32f10x.h"

void LED_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = LED_PORT_MODE;
    GPIO_InitStructure.GPIO_Pin = LED_PORT_PIN;
    GPIO_InitStructure.GPIO_Speed = LED_PORT_SPEED;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);

    LED_OFF();
}

void LED_ON(void)
{
    GPIO_SetBits(LED_PORT, LED_PORT_PIN);
}

void LED_OFF(void)
{
    GPIO_ResetBits(LED_PORT, LED_PORT_PIN);
}
