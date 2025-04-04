#ifndef USART_H
#define USART_H

#include "stm32f4xx.h"

extern volatile uint8_t rx_data;

void usart_init(uint32_t baudRate);
void usart_send(uint8_t data);
void usart_send_str(char *str);
void usart_send_uint32(uint32_t num);

void _usart_pin_init(void);
void _usart_conf_init(uint32_t baudRate);
void _usart_it_init(void);


#endif
