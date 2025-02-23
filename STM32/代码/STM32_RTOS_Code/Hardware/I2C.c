#include "I2C.h"
#include "Delay.h"
#include "OLED.h"

#ifdef EXP_I2C

#define SCL_PORT GPIOB
#define SCL_PIN GPIO_Pin_0

#define SDA_PORT GPIOB
#define SDA_PIN GPIO_Pin_1

void Soft_SCL_Write(uint8_t bitVal)
{
    GPIO_WriteBit(SCL_PORT, SCL_PIN, (BitAction)bitVal);
    Delay_us(10);
}

void Soft_SDA_Write(uint8_t bitVal)
{
    GPIO_WriteBit(SDA_PORT, SDA_PIN, (BitAction)bitVal);
    Delay_us(10);
}

uint8_t Soft_SDA_Read(void)
{
    uint8_t res = GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN);
    Delay_us(10);
    return res;
}

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
    Soft_SCL_Write(1); // 拉高SCL
    Soft_SDA_Write(1); // 拉高SDA

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
    起始条件：SCL高电平期间，SDA从高电平切换到低电平
*/
void I2C_Soft_Start(void)
{
    Soft_SCL_Write(1); // 起使条件开始时保证SCL处于高电平
    Soft_SDA_Write(1); // 保证SDA处于高电平
    Soft_SDA_Write(0); // 构造起始条件，即在SCL高电平期间拉低SDA
    Soft_SCL_Write(0); // 起始条件结束后，主机务必拉低SCL占用总线，防止其他I2C设备控制总线
}

/*
    终止条件：SCL高电平期间，SDA从低电平切换到高电平
*/
void I2C_Soft_End(void)
{
    Soft_SDA_Write(0); // 确保SDA在低电平，这一步必须在SCL拉高之前，因为SCL高电平期间不能拉低SDA，否则就会构成起使条件
    Soft_SCL_Write(1); // 确保SCL在高电平
    Soft_SDA_Write(1); // 构造终止条件，在SCL高电平期间，拉高SDA
}

/*
    准备一位数据，并等待从机读取
*/
void _I2C_Soft_SendBit(uint8_t bitVal)
{
    Soft_SDA_Write(bitVal);
    // 拉高SCL，从机读取数据
    Soft_SCL_Write(1);
    // 拉低SCL，主备下一个数据
    Soft_SCL_Write(0);
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
    // 释放SDA线
    Soft_SDA_Write(1);

    // 拉高SCL，并读取
    Soft_SCL_Write(1);
    uint8_t bit = Soft_SDA_Read();
    Soft_SCL_Write(0);
    return bit;
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

void I2C_Soft_SendAck(uint8_t ackBit)
{
    Soft_SDA_Write(ackBit);
    Soft_SCL_Write(1);
    Soft_SCL_Write(0);
}

uint8_t I2C_Soft_ReceiveAck(void)
{
    uint8_t ackBit = 0;
    Soft_SDA_Write(1); // 释放SDA线
    Soft_SCL_Write(1); // 拉高SCL准备读取
    ackBit = Soft_SDA_Read();
    Soft_SCL_Write(0);
    return ackBit;
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
