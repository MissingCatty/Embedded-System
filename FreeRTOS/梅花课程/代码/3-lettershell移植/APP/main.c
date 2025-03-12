#include "stm32f4xx.h"
#include "main.h"

int main(void)
{
    board_init();
    usart_init(9600);
    myshell_init();
		
    while (1)
    {
       shellTask(&shell);
    }
}
