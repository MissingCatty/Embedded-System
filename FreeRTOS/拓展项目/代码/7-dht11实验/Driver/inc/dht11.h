#ifndef DHT11_H
#define DHT11_H

#include <stm32f4xx.h>

extern uint8_t dht11_humid_integer;
extern uint8_t dht11_humid_decimal;
extern uint8_t dht11_temp_integer;
extern uint8_t dht11_temp_decimal;

void    dht11_init(void);
uint8_t dht11_receive_data(void);

#endif
