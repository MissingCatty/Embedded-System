#include "stm32f4xx.h"
#include "main.h"
#include "stdio.h"

int main(void)
{
		MyUSART_Init(115200);
    delay_init();
    LCD_Init();
    LCD_Clear(RED);
    delay_ms(1000);
    LCD_Clear(~RED);
    delay_ms(1000);

    dht11_init();

    char msg[64];
    while (1)
    {
        dht11_receive_data();
        sprintf(msg, "T: %d, H: %d", dht11_temp_integer, dht11_humid_integer);
        LCD_ShowString(0, 0, 240, 30, 24, msg);
				printf("Humid: %d\n", dht11_temp_integer);
        printf("Temp: %d\n", dht11_humid_integer);
        delay_s(1);
    }
}
