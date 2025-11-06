#include "driver.h"

led_conf_t led0 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_5,
    GPIOE,
    Bit_RESET,
    Bit_SET
};

led_conf_t led1 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_6,
    GPIOE,
    Bit_RESET,
    Bit_SET
};
