#include "board.h"

led_conf_t led0 = {
    .rcc = RCC_AHB1Periph_GPIOE,
    .port       = GPIOE,
    .pin        = GPIO_Pin_5,
    .on_level   = Bit_RESET,
    .off_level  = Bit_SET,
};

led_conf_t led1 = {
    .rcc = RCC_AHB1Periph_GPIOE,
    .port       = GPIOE,
    .pin        = GPIO_Pin_6,
    .on_level   = Bit_RESET,
    .off_level  = Bit_SET
};

led_conf_t led2 = {
    .rcc = RCC_AHB1Periph_GPIOC,
    .port       = GPIOC,
    .pin        = GPIO_Pin_13,
    .on_level   = Bit_RESET,
    .off_level  = Bit_SET
};
