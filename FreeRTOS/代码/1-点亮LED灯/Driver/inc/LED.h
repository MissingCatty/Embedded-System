#include "stm32f4xx.h"

/*
    LED0: PE5
    LED1: PE6
    LED2: PC13
*/
#define LED_PORT GPIOE
#define LED0 GPIO_Pin_5
#define LED1 GPIO_Pin_6

void LED_Config(uint16_t LEDx);
void LED_ON(uint16_t LEDx);
void LED_OFF(uint16_t LEDx);
