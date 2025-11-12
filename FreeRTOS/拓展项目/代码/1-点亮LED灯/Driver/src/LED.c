#include "LED.h"

void LED_Delay(void)
{
    for (uint32_t a = 0; a < 1000; a++)
    {
        for (uint32_t b = 0; b < 1000; b++)
        {
            __NOP();
            __NOP();
            __NOP();
            __NOP();
        }
    }
}

#ifdef DEFINE_MODE

void LED_Init(void)
{
    RCC_AHB1PeriphClockCmd(LED0_RCC, ENABLE);
    RCC_AHB1PeriphClockCmd(LED1_RCC, ENABLE);
    RCC_AHB1PeriphClockCmd(LED2_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin   = LED0_PIN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; // 用于配置输入模式或开漏模式下的上拉/下拉状态
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(LED0_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin   = LED1_PIN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; // 用于配置输入模式或开漏模式下的上拉/下拉状态
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(LED1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin   = LED2_PIN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; // 用于配置输入模式或开漏模式下的上拉/下拉状态
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(LED2_PORT, &GPIO_InitStructure);
}

#endif

#ifdef OBJECT_ORIENTED_MODE

void LED_Init(const led_conf_t * led)
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

void LED_DeInit(const led_conf_t * led)
{
    GPIO_DeInit(led->port);
}

void LED_ON(const led_conf_t * led)
{
    GPIO_WriteBit(led->port, led->pin, led->on_level);
}

void LED_OFF(const led_conf_t * led)
{
    GPIO_WriteBit(led->port, led->pin, led->off_level);
}

#endif
