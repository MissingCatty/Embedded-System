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


timer_base_conf_t tim3 = {
    RCC_APB1Periph_TIM3,
    TIM3,
    8400 - 1,
    TIM_CounterMode_Up,
    10000 - 1,
    TIM_CKD_DIV1,
    0
};

timer_base_conf_t tim4 = {
    RCC_APB1Periph_TIM4,
    TIM4,
    8400 - 1,
    TIM_CounterMode_Up,
    10000 - 1,
    TIM_CKD_DIV1,
    0
};
