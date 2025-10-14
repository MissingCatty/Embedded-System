#ifndef DELAY_H
#define DELAY_H

#include <stm32f4xx.h>

void delay_init(void);

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void delay_s(uint32_t s);

#endif
