#include "stm32f4xx.h"
#include "main.h"

int main(void)
{
    delay_init();
    led_init(&led0);
    while (1)
    {
        led_on(&led0);
        delay_s(1);
        led_off(&led0);
        delay_s(1);
    }
}
