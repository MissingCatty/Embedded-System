#ifndef TIMER_H
#define TIMER_H

#include <stm32f4xx.h>

void timer_init(void);
uint64_t timer_now_us(void);
uint64_t timer_now_ms(void);
uint64_t timer_now_s(void);
void timer_delay_us(uint32_t us);
void timer_delay_ms(uint32_t ms);
void timer_delay_s(uint32_t s);

#endif
