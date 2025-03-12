#include "led.h"

led_conf_t led0 = {
    .rcc       = RCC_AHB1Periph_GPIOE,
    .port      = GPIOE,
    .pin       = GPIO_Pin_5,
    .off_level = Bit_SET,
    .on_level  = Bit_RESET
};
