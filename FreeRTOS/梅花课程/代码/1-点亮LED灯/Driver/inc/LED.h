#ifndef LED_H
#define LED_H

#include "stm32f4xx.h"
#include "func_selection.h"

/*
    LED0: PE5
    LED1: PE6
    LED2: PC13
*/

void LED_Delay(void);

#ifdef DEFINE_MODE

#define LED0_RCC   RCC_AHB1Periph_GPIOE
#define LED0_PORT  GPIOE
#define LED0_PIN   GPIO_Pin_5
#define LED0_ON()  GPIO_WriteBit(LED0_PORT, LED0_PIN, Bit_RESET)
#define LED0_OFF() GPIO_WriteBit(LED0_PORT, LED0_PIN, Bit_SET)

#define LED1_RCC   RCC_AHB1Periph_GPIOE
#define LED1_PORT  GPIOE
#define LED1_PIN   GPIO_Pin_6
#define LED1_ON()  GPIO_WriteBit(LED1_PORT, LED1_PIN, Bit_RESET)
#define LED1_OFF() GPIO_WriteBit(LED1_PORT, LED1_PIN, Bit_SET)

#define LED2_RCC   RCC_AHB1Periph_GPIOC
#define LED2_PORT  GPIOC
#define LED2_PIN   GPIO_Pin_13
#define LED2_ON()  GPIO_WriteBit(LED2_PORT, LED2_PIN, Bit_RESET)
#define LED2_OFF() GPIO_WriteBit(LED2_PORT, LED2_PIN, Bit_SET)

void LED_Init(void);

#endif

#ifdef OBJECT_ORIENTED_MODE

typedef struct {
    uint32_t      rcc;
    uint16_t      pin;
    GPIO_TypeDef *port;
    BitAction     on_level;
    BitAction     off_level;
} led_conf_t; // _t表示是一个typedef过来的东西

void LED_Init(const led_conf_t *led);
void LED_DeInit(const led_conf_t *led);
void LED_ON(const led_conf_t *led);
void LED_OFF(const led_conf_t *led);

#endif

#endif
