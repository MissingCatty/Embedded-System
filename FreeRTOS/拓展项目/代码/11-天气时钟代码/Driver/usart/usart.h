#ifndef USART_H
#define USART_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ringbuffer8.h"

extern ringbuffer8_t     rb;
extern SemaphoreHandle_t xUsartRxSemaphore;
extern SemaphoreHandle_t xUsartTxSemaphore;

void usart_init(uint32_t baudRate);
void usart_send(const char *str);

void _usart_pin_init(void);
void _usart_conf_init(uint32_t baudRate);
void _usart_it_init(void);
void _usart_dma_init(void);

#endif
