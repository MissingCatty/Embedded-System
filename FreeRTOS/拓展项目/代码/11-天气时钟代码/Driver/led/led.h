#ifndef LED_H
#define LED_H

#include "stm32f4xx.h"

/*
    LED0: PE5
    LED1: PE6
    LED2: PC13
*/
typedef struct {
    uint32_t      rcc;
    uint16_t      pin;
    GPIO_TypeDef *port;
    BitAction     on_level;
    BitAction     off_level;
} led_conf_t; // _t表示是一个typedef过来的东西

void led_init(const led_conf_t *led);
void led_deInit(const led_conf_t *led);
void led_on(const led_conf_t *led);
void led_off(const led_conf_t *led);

#endif
