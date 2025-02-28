#include "SPI.h"

#ifdef EXP_SPI
/*
    SPI1
        - SPI1_NSS: PA4
        - SPI1_SCK: PA5
        - SPI1_MISO: PA6
        - SPI1_MOSI: PA7

    SPI2
        - SPI2_NSS: PB12
        - SPI2_SCK: PB13
        - SPI2_MISO: PB14
        - SPI2_MOSI: PB15
*/

void SPI_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); // 72MHz
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); // 36MHz
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    /*
        ==========================SPI1配置==========================
    */

    // SCK, MOSI配置成复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // NSS配置成推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MISO配置成浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                      // 主模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  // 数据单元8b
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                         // SCK空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                       // 在SCK第一个边沿读取
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          // 软件片选
    SPI_InitStructure.SPI_BaudRatePrescaler =
        SPI_BaudRatePrescaler_256;                     // f_PCLK / f_prescaler
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // 先发送最高位
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);

    /*
        ==========================SPI2配置==========================
    */
    // NSS, SCK, MOSI配置成浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MISO配置成复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;                       // 主模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  // 数据单元8b
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                         // SCK空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                       // 在SCK第一个边沿读取
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          // 硬件片选
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 // 先发送最高位
    // 从机不需要配置SPI_BaudRatePrescaler，因为时钟由主机提供
    SPI_Init(SPI2, &SPI_InitStructure);

    SPI_Cmd(SPI2, ENABLE);
}

void MASTER_NSS_WRITE(uint8_t bitval)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)bitval);
}

void Master_Send_Byte(uint8_t byte)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET)
        ;
    SPI_I2S_SendData(SPI1, byte);
}

uint16_t Slave_Read_Byte(void)
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) != SET)
        ;
    return SPI_I2S_ReceiveData(SPI2);
}

#endif
