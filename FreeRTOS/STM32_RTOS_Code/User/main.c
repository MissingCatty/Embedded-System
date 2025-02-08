#include "stm32f10x.h" // Device header
#include "Experiment.h"

int main(void)
{
#ifdef EXP_GPIO_LIGHTSENSOR
	Exp_GPIO_LightSensor();
#endif

#ifdef EXP_TIM_IT_UPDATE
	Exp_TIM_IT_Update();
#endif

#ifdef EXP_TIM_PWM
	Exp_TIM_PWM();
#endif

#ifdef EXP_TIM_IC
	Exp_TIM_IC();
#endif

#ifdef EXP_ADC_DMA
	Exp_ADC_DMA();
#endif

#ifdef EXP_USART
	Exp_USART();
#endif

#ifdef EXP_I2C
	Exp_I2C();
#endif

#ifdef EXP_SPI
	Exp_SPI();
#endif // DEBUG
}
