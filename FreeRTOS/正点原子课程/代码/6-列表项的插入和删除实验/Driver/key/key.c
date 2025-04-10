#include "key.h"
#include "driver.h"

void key_init(key_conf_t *keyx)
{
    RCC_AHB1PeriphClockCmd(keyx->key_rcc_colck, ENABLE); // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = keyx->key_gpio_pin;  // 设置引脚
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;        // 输入模式
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;        // 上拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;   // 中速
    GPIO_Init(keyx->key_gpio_port, &GPIO_InitStructure); // 初始化GPIO

    EXTI_InitTypeDef EXTI_InitStructure;
    SYSCFG_EXTILineConfig(keyx->key_exit_port_source, keyx->key_exit_pin_source); // 选择引脚
    EXTI_InitStructure.EXTI_Line    = keyx->key_exit_line;                        // 选择线路
    EXTI_InitStructure.EXTI_Mode    = keyx->key_exit_mode;                        // 中断模式
    EXTI_InitStructure.EXTI_Trigger = keyx->key_exit_trigger;                     // 上下沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;                                     // 使能中断
    EXTI_Init(&EXTI_InitStructure);                                               // 初始化中断

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = keyx->key_nvic_irq_channel;         // 中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = keyx->key_nvic_preemption_priority; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = keyx->key_nvic_sub_priority;        // 响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;                             // 使能中断
    NVIC_Init(&NVIC_InitStructure);                                                            // 初始化中断
}

uint8_t key_pushed(key_conf_t *keyx)
{
    if (GPIO_ReadInputDataBit(keyx->key_gpio_port, keyx->key_gpio_pin) == keyx->key_pushed)
        return 1; // 按键按下
    return 0;     // 按键未按下
}

