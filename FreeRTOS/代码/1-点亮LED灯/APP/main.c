#include "stm32f4xx.h"
#include "LED.h"

int main(void)
{
	LED_Config(LED0);
	LED_ON(LED0);

	while (1)
	{
		;
	}
}
