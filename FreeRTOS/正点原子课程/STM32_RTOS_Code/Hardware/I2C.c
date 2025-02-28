#include "I2C.h"
#include "Delay.h"
#include "OLED.h"

#ifdef EXP_I2C

#define SCL_PORT GPIOB
#define SCL_PIN GPIO_Pin_0
#define SCL_WRITE(status) GPIO_WriteBit(SCL_PORT, SCL_PIN, (BitAction)status)
#define SCL_READ() GPIO_ReadInputDataBit(SCL_PORT, SCL_PIN)

#define SDA_PORT GPIOB
#define SDA_PIN GPIO_Pin_1
#define SDA_WRITE(status) GPIO_WriteBit(SDA_PORT, SDA_PIN, (BitAction)status)
#define SDA_READ() GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN)

void I2C_Config(void)
{
    /*
        ===================软件模拟主机===================
    */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin = SCL_PIN | SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SCL_PORT, &GPIO_InitStructure);

    // 初始化SCL和SDA
	SCL_WRITE(1); // 拉高SCL
    SDA_WRITE(1); // 拉高SDA
	
    /*
        ===================I2C2从机配置===================
    */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;                                // I2C 工作模式
    I2C_InitStructure.I2C_OwnAddress1 = 0x80;                                 // 设置从设备地址
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;                        // 50% 占空比
    I2C_InitStructure.I2C_ClockSpeed = 100000;                                // 设置时钟频率为 100 kHz
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 7 位地址
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;                               // 启用应答
    I2C_Init(I2C2, &I2C_InitStructure);

    I2C_Cmd(I2C2, ENABLE);

    /*
        ===================I2C2从机中断配置===================
    */
    I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}

/*
    起始条件
*/
void I2C_Soft_Start(void)
{
	SCL_WRITE(1);
    SDA_WRITE(1);
    Delay_us(10);
    SDA_WRITE(0);
    Delay_us(10);
}

/*
    终止条件
*/
void I2C_Soft_End(void)
{
	SCL_WRITE(1);
    SDA_WRITE(0);
    Delay_us(10);
    SDA_WRITE(1);
    Delay_us(10);
}

/*
    准备一位数据，并等待从机读取
*/
void _I2C_Soft_SendBit(uint8_t bitVal)
{
    // 拉低SCL，准备数据
    SCL_WRITE(0);
    Delay_us(10);
    SDA_WRITE(bitVal);
    Delay_us(10);

    // 拉高SCL，从机读取数据
    SCL_WRITE(1);
    Delay_us(20);
}

/*
    发送一个字节的数据，高位先发
*/
void I2C_Soft_SendByte(uint8_t byte)
{
    uint8_t mask = 0x80;
    for (int i = 0; i < 8; i++)
    {
        _I2C_Soft_SendBit((byte & mask) >> (7 - i));
        mask = mask >> 1;
    }
}

/*
    接收一位数据
*/
uint8_t _I2C_Soft_ReceiveBit(void)
{
    // 释放SDA线，并将SCL线拉低，让从设备准备数据
    SDA_WRITE(1);
    SCL_WRITE(0);
    Delay_us(20);

    // 拉高SCL，并读取
    SCL_WRITE(1);
    Delay_us(20);
    return SDA_READ();
}

/*
    接收一个字节
*/
uint8_t I2C_Soft_ReceiveByte(void)
{
    uint8_t byte = 0x00;
    for (int i = 0; i < 8; i++)
    {
        uint8_t bit = _I2C_Soft_ReceiveBit();
        byte = byte ^ (bit << (7 - i));
    }
    return byte;
}

/*
    I2C中断服务函数
*/
void I2C2_EV_IRQHandler(void)
{
    if (I2C_GetITStatus(I2C2, I2C_IT_RXNE) != RESET)
    {
        OLED_ShowString(4, 1, "[MESSAGE]:");
        OLED_ShowBinNum(4, 11, I2C2->DR, 2);
    }
}
#endif // DEBUG
