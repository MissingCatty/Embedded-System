#include "stm32f4xx.h"
#include "main.h"

int main(void)
{
#ifdef DEFINE_MODE
    LED_Init();

    while (1)
    {
        LED0_ON();
        LED1_ON();
        LED2_ON();
        LED_Delay();

        LED0_OFF();
        LED1_OFF();
        LED2_OFF();
        LED_Delay();
    }
#endif

#ifdef OBJECT_ORIENTED_MODE
    LED_Init(&led0);
    LED_Init(&led1);
    LED_Init(&led2);

    while (1)
    {
        LED_ON(&led0);
        LED_ON(&led1);
        LED_ON(&led2);
        LED_Delay();

        LED_OFF(&led0);
        LED_OFF(&led1);
        LED_OFF(&led2);
        LED_Delay();
    }

#endif
}
