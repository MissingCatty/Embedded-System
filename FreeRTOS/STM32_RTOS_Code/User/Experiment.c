#include "stm32f10x.h"
#include "Experiment.h"
#include "LED.h"
#include "LightSensor.h"
#include "Timer.h"
#include "OLED.h"

#ifdef EXP_GPIO_LIGHTSENSOR
void Exp_GPIO_LightSensor(void)
{
    LED_Init();
    LightSensor_Init();

    while (1)
    {
        // 读取光敏传感器的DO
        uint8_t data = GPIO_ReadInputDataBit(LightSensor_PORT, LightSensor_PORT_PIN);

        // 如果是1，说明光线暗，打开LED
        if (data)
        {
            LED_ON();
        }
        else
        {
            LED_OFF();
        }
    }
}
#endif

#ifdef EXP_TIM_IT_UPDATE
void Exp_TIM_IT_Update(void)
{
    OLED_Init();
    Timer_Init();

    while (1)
    {
        OLED_ShowString(1, 1, "second:");
        OLED_ShowNum(1, 9, Timer_GetSeconds(), 3);
    }
}
#endif

#ifdef EXP_TIM_PWM
void Exp_TIM_PWM(void)
{
    Timer_Init();

    while (1)
    {
    }
}
#endif

#ifdef EXP_TIM_IC
void Exp_TIM_IC(void)
{
    Timer_Init();
    OLED_Init();

    while (1)
    {
        OLED_ShowString(1, 1, "frequency:");
        OLED_ShowNum(1, 11, TIM_GetFrequency(), 3);
        float duty_ratio = TIM_GetDutyRatio();
    }
}
#endif
