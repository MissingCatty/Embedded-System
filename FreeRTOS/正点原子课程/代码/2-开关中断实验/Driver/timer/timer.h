#ifndef TIMER_H
#define TIMER_H

#include "stm32f4xx.h"

void timer_init(void);

void _timer_cmd(void);
void _timer_it_cmd(void);
void _timer_base_init(void);

#endif
