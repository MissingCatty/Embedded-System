#ifndef KEY_H
#define KEY_H

#include "stm32f4xx.h"

#define KEY_INTERRUPT_ENABLE 1 // 是否使能按键中断

typedef struct
{
    uint32_t      key_rcc_colck;
    uint16_t      key_gpio_pin;  // GPIO引脚
    GPIO_TypeDef *key_gpio_port; // GPIO端口
    uint8_t       key_pushed;

    uint8_t key_exit_port_source;
    uint8_t key_exit_pin_source;
    uint32_t key_exit_line;
    EXTIMode_TypeDef key_exit_mode;
    EXTITrigger_TypeDef key_exit_trigger;

    uint8_t key_nvic_irq_channel;
    uint8_t key_nvic_preemption_priority;
    uint8_t key_nvic_sub_priority;
} key_conf_t;

void key_init(key_conf_t *keyx);

uint8_t key_pushed(key_conf_t *keyx);

#endif // !KEY_H
