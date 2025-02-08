#include "stm32f10x.h"
#include "Experiment.h"
#include "LED.h"
#include "LightSensor.h"
#include "Timer.h"
#include "OLED.h"
#include "ADC.h"
#include "DMA.h"
#include "USART.h"
#include "Key.h"
#include "I2C.h"
#include "SPI.h"

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

#ifdef EXP_ADC_DMA
void Exp_ADC_DMA(void)
{
    OLED_Init();
    Timer_Init();
    ADC_Config();
    DMA_Config();

    uint16_t *SRAM_Addr = DMA_GetResAddr();

    while (1)
    {
        OLED_ShowString(1, 1, "CH1:");
        OLED_ShowNum(1, 5, SRAM_Addr[0], 4);

        OLED_ShowString(2, 1, "CH2:");
        OLED_ShowNum(2, 5, SRAM_Addr[1], 4);

        OLED_ShowString(3, 1, "CH3:");
        OLED_ShowNum(3, 5, SRAM_Addr[2], 4);
    }
}
#endif

#ifdef EXP_USART
void Exp_USART(void)
{
    USART_Config();
    Key_Config();
    OLED_Init();

    uint8_t *send_buf = USART_Get_Send_Buffer();
    uint8_t send_buf_size = USART_Get_Send_Buffer_Length();
    uint8_t *rec_buf = USART_Get_Receive_Buffer();
    uint8_t rec_buf_size = USART_Get_Receive_Buffer_Length();

    while (1)
    {
        OLED_ShowString(1, 1, "TX:");
        for (int i = 0; i < send_buf_size; i++)
        {
            OLED_ShowHexNum(1, 4 + 3 * i, send_buf[i], 2);
        }
        OLED_ShowString(2, 1, "RX:");
        for (int i = 0; i < rec_buf_size; i++)
        {
            OLED_ShowHexNum(2, 4 + 3 * i, rec_buf[i], 2);
        }
    }
}
#endif

#ifdef EXP_I2C
void Exp_I2C(void)
{
    OLED_Init();
    I2C_Config();

    I2C_Soft_Start();
    I2C_Soft_SendByte(0x80);
    I2C_Soft_End();

    while (1)
    {
    }

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // GPIO_InitTypeDef GPIO_InitStructure;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOB, &GPIO_InitStructure);

    // OLED_Init();

    // GPIO_WriteBit(GPIOB, GPIO_Pin_0, (BitAction)1);
    // GPIO_WriteBit(GPIOB, GPIO_Pin_1, (BitAction)0);

    // while (1)
    // {
    //     OLED_ShowHexNum(1, 1, GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0), 2);
    //     OLED_ShowHexNum(2, 1, GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_1), 2);
    // }
}
#endif

#ifdef EXP_SPI
void Exp_SPI(void)
{
    OLED_Init();
    SPI_Config();

    MASTER_NSS_WRITE(0);
    Master_Send_Byte(0xff);
    uint8_t data = Slave_Read_Byte();

    while (1)
    {
        OLED_ShowHexNum(1, 1, data, 2);
    }
}
#endif
