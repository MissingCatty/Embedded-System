#include "usart.h"
#include "ringbuffer8.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256

uint8_t       buffer[BUFFER_SIZE];
ringbuffer8_t rb;

SemaphoreHandle_t xUsartRxSemaphore;
SemaphoreHandle_t xUsartTxSemaphore;

void usart_init(uint32_t baudRate)
{
    _usart_pin_init();
    _usart_conf_init(baudRate);
    _usart_it_init();
    _usart_dma_init();
    rb                = rb8_new(buffer, BUFFER_SIZE);
    xUsartRxSemaphore = xSemaphoreCreateBinary();
    xUsartTxSemaphore = xSemaphoreCreateBinary();
}

void _usart_pin_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); // 注意这里是GPIO_PinSource10，而不是GPIO_Pin_10

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void _usart_conf_init(uint32_t baudRate)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = baudRate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStructure);

    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

void _usart_it_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel    = DMA2_Stream7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
}

void _usart_dma_init(void)
{
    // 使能DMA2时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    // DMA配置
    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_Channel            = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_MemoryToPeripheral;  // 从内存到外设
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;   // 外设地址不变
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据宽度为8位
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;        // 内存地址递增
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;     // 内存数据宽度为8位
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;             // 非循环模式
    DMA_InitStructure.DMA_Priority           = DMA_Priority_Low;            // DMA优先级为低
    DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Enable;         // 启用FIFO提高传输效率
    DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;      // FIFO全满时开始传输
    DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_INC8;        // 一次性传输8个数据单元，配合FIFO进一步提高效率
    DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;  // 外设突发传输为单次传输
    DMA_Init(DMA2_Stream7, &DMA_InitStructure);

    // DMA中断配置
    DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
}

void usart_send(const char *str)
{
    int32_t len = strlen(str);

    int32_t chunk_size = 65535;

    // 如果数据过长，就分块发送
    do
    {
        len = len < chunk_size ? len : chunk_size;

        // 准备发送
        DMA2_Stream7->M0AR = (uint32_t)str;
        DMA2_Stream7->NDTR = len;
        DMA_Cmd(DMA2_Stream7, ENABLE);

        // 等待发送完毕
        xSemaphoreTake(xUsartTxSemaphore, portMAX_DELAY);

        // 后续数据处理
        len -= chunk_size;
        str += chunk_size;
    } while (len > 0);

    USART_ClearFlag(USART1, USART_FLAG_TC);
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        if (!rb8_full(rb))
        {
            uint16_t rx_data = USART_ReceiveData(USART1); // 9位数据
            rb8_put(rb, (uint8_t)rx_data);                // 强制截断为8位
        } else
        {
            printf("[ringbuffer error]: ringbuffer is full.");
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        BaseType_t        xHigherPriorityTaskWoken = pdFALSE;
        volatile uint16_t dummy                    = USART_ReceiveData(USART1); // 读取DR数据寄存器以清除IDLE标志，volatile防止编译器优化掉这句
        if (!rb8_empty(rb))
        {
            xSemaphoreGiveFromISR(xUsartRxSemaphore, &xHigherPriorityTaskWoken); // 如果give后，有比当前更高优先级的任务就绪，则变为true
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        USART_ClearITPendingBit(USART1, USART_IT_IDLE);
    }
}

void DMA2_Stream7_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xUsartTxSemaphore, &xHigherPriorityTaskWoken); // 开锁
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
    }
}
