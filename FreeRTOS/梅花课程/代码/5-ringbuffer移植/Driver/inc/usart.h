#ifndef USART_H
#define USART_H

#include "stm32f4xx.h"

void usart_init(uint32_t baudRate);
void usart_send(uint8_t data);

void _usart_pin_init(void);
void _usart_conf_init(uint32_t baudRate);
void _usart_it_init(void);


#endif
