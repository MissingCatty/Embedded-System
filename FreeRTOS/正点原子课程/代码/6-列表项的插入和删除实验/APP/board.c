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

key_conf_t key3 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_1,
    GPIOE,
    0,

    EXTI_PortSourceGPIOE,
    EXTI_PinSource1,
    EXTI_Line1,
    EXTI_Mode_Interrupt,
    EXTI_Trigger_Falling,

    EXTI1_IRQn,
    5,
    0
};

key_conf_t key4 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_0,
    GPIOE,
    0,
    EXTI_PortSourceGPIOE,
    EXTI_PinSource0,
    EXTI_Line0,
    EXTI_Mode_Interrupt,
    EXTI_Trigger_Falling,
    EXTI0_IRQn,
    5,
    0
};

