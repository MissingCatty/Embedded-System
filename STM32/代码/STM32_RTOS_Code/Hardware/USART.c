#include "USART.h"

#ifdef EXP_USART

#define USART_SEND_BUFFERSIZE 4
uint8_t USART_Send_Buffer[USART_SEND_BUFFERSIZE];
#define USART_RECEIVE_BUFFERSIZE 4
uint8_t USART_Receive_Buffer[USART_RECEIVE_BUFFERSIZE];

void USART_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // USART1_TX
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;       // PA9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART1_InitStructure;
    USART1_InitStructure.USART_BaudRate = 9600;
    USART1_InitStructure.USART_Mode = USART_Mode_Tx;
    USART1_InitStructure.USART_Parity = USART_Parity_No;
    USART1_InitStructure.USART_StopBits = USART_StopBits_1;
    USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART1_InitStructure);

    USART_Cmd(USART1, ENABLE);

    // USART2_RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;             // PA3
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART2_InitStructure;
    USART2_InitStructure.USART_BaudRate = 9600;
    USART2_InitStructure.USART_Mode = USART_Mode_Rx;
    USART2_InitStructure.USART_Parity = USART_Parity_No;
    USART2_InitStructure.USART_StopBits = USART_StopBits_1;
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART2_InitStructure);

    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);

    USART_Cmd(USART2, ENABLE);

    // USART2使用DMA配置
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                          // 连续模式，计数器到0后自动重装，并且重新装载源地址和目的地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                       // 传输方向：外设to内存
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);      // 外设寄存器地址
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  // 外设数据宽度，8位
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;         // 外设地址不自增
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(USART_Receive_Buffer); // 内存地址
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;          // 内存数据宽度，8位
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                  // 内存地址自增
    DMA_InitStructure.DMA_BufferSize = USART_SEND_BUFFERSIZE;                // 传输4次
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                             // 硬件触发
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel6, ENABLE);
}

#include <stdlib.h>
#include <time.h>
/*
    随机生成发送缓冲区中的内容并逐个发送
    返回值: 发送缓冲区地址
*/
void USART_Generate_And_Send(void)
{
    for (int i = 0; i < USART_SEND_BUFFERSIZE; i++)
    {
        USART_Send_Buffer[i] = rand();
        USART_SendByte(USART1, USART_Send_Buffer[i]);
    }
}

/*
    返回发送缓冲区地址
*/
uint8_t *USART_Get_Send_Buffer(void)
{
    return USART_Send_Buffer;
}

/*
    返回发送缓冲区长度
*/
uint8_t USART_Get_Send_Buffer_Length(void)
{
    return USART_SEND_BUFFERSIZE;
}

/*
    返回接收缓冲区地址
*/
uint8_t *USART_Get_Receive_Buffer(void)
{
    return USART_Receive_Buffer;
}

/*
    返回接收缓冲区长度
*/
uint8_t USART_Get_Receive_Buffer_Length(void)
{
    return USART_RECEIVE_BUFFERSIZE;
}

/*
    USART的发送函数
*/
void USART_SendByte(USART_TypeDef *USARTx, uint8_t byte)
{
    // 等待发送寄存器空
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) != SET)
        ;

    USART_SendData(USARTx, byte);
}

/*
    USART的接收函数
*/
uint8_t USART_ReadByte(USART_TypeDef *USARTx)
{
    while (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) != SET)
        ;

    return USART_ReceiveData(USARTx);
}
#endif
