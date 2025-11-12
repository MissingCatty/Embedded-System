#include "led.h"

void led_init(const led_conf_t * led)
{
    RCC_AHB1PeriphClockCmd(led->rcc, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin   = led->pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(led->port, &GPIO_InitStructure);

    GPIO_WriteBit(led->port, led->pin, led->off_level);
}

void led_deInit(const led_conf_t * led)
{
    GPIO_DeInit(led->port);
}

void led_on(const led_conf_t * led)
{
    GPIO_WriteBit(led->port, led->pin, led->on_level);
}

void led_off(const led_conf_t * led)
{
    GPIO_WriteBit(led->port, led->pin, led->off_level);
}
