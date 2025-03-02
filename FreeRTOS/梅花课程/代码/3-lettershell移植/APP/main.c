#include "stm32f4xx.h"
#include "main.h"

int main(void)
{
    board_init();
    usart_init(115200);
    myshell_init();
		
    while (1)
    {
       shellTask(&shell);
    }
}
