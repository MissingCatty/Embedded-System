#include "esp_at.h"
#include <stm32f4xx.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "driver.h"
// USART2_RX: PA3
// USART2_TX: PA2

#define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))

#define ESP_RX_BUFFER_SIZE 4096
uint8_t       esp_rx_buffer[ESP_RX_BUFFER_SIZE];
ringbuffer8_t esp_rb;

static SemaphoreHandle_t xEspUartTxSemaphore;
static SemaphoreHandle_t xEspUartRxSemaphore;

esp_at_ack_t esp_at_ack;

uint8_t  ack_info_buffer[256];                // 信息接收缓冲区
uint8_t *p_ack_info_buffer = ack_info_buffer; // 接收缓冲区起始指针

esp_wifi_state_t esp_wifi_state;

esp_at_ack_match_t esp_at_ack_match_table[] = {
    {ESP_AT_ACK_OK, "OK\r\n"},
    {ESP_AT_ACK_ERROR, "ERROR\r\n"},
    {ESP_AT_ACK_BUSY, "busy p...\r\n"},
    {ESP_AT_ACK_READY, "ready\r\n"},
    {ESP_AT_ACK_NONE, NULL}
};

void _esp_uart_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel    = DMA1_Stream6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    // 使能DMA1时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    // DMA配置
    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_Channel            = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
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
    DMA_Init(DMA1_Stream6, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
}

void esp_at_init(void)
{
    _esp_uart_init();
    esp_rb              = rb8_new(esp_rx_buffer, ESP_RX_BUFFER_SIZE);
    xEspUartTxSemaphore = xSemaphoreCreateBinary();
    xEspUartRxSemaphore = xSemaphoreCreateBinary();
}

esp_at_ack_t esp_match_from_ack(const char *ack)
{
    uint16_t size = ARRAY_SIZE(esp_at_ack_match_table);
    for (uint16_t i = 0; i < size; i++)
    {
        if (!strcmp(ack, esp_at_ack_match_table[i].str))
        {
            return esp_at_ack_match_table[i].ack;
        }
    }
    return ESP_AT_ACK_NONE;
}

void esp_at_send(const char str[])
{
    uint32_t len       = strlen(str);
    DMA1_Stream6->M0AR = (uint32_t)str;
    DMA1_Stream6->NDTR = len;

    DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
    DMA_Cmd(DMA1_Stream6, ENABLE);
    xSemaphoreTake(xEspUartTxSemaphore, portMAX_DELAY);
    USART_ClearFlag(USART2, USART_FLAG_TC);
}

esp_at_ack_t esp_at_send_cmd(const char cmd[], uint16_t timeout)
{
    p_ack_info_buffer = ack_info_buffer; // 发送指令之前先把返回信息的指针复原
    esp_at_send(cmd);

    if (xSemaphoreTake(xEspUartRxSemaphore, pdMS_TO_TICKS(timeout)) == pdTRUE)
    {
        printf("%d", (uint8_t)esp_at_ack);
        return esp_at_ack;
    }
    printf("0");
    return ESP_AT_ACK_NONE;
}

void esp_wifi_init(void)
{
    esp_at_send_cmd("AT+CWMODE=1\r\n", 2000);
}

bool _esp_parse_wifi_cwstate(char *str)
{
    str = strstr(str, "+CWSTATE:");
    if (str == NULL)
    {
        return false;
    }
    if (sscanf(str, "+CWSTATE:%d,\"%63[^\"]", &esp_wifi_state.state, &esp_wifi_state.ssid) == 2)
    {
        return true;
    }
    return false;
}

bool esp_wifi_connected(void)
{
    if (esp_at_send_cmd("AT+CWSTATE?\r\n", 2000) == ESP_AT_ACK_OK)
    {
        _esp_parse_wifi_cwstate((char *)ack_info_buffer);
        if (esp_wifi_state.state == 2)
        {
            return true;
        }
    }
    return false;
}

bool esp_at_wifi_connect(const char ssid[], const char pwd[])
{
    char cmd[128];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    printf("%s", cmd);
    if (esp_at_send_cmd(cmd, 20000) == ESP_AT_ACK_OK)
    {
        esp_at_send_cmd("AT+CWJAP?\r\n", 2000);
        _esp_parse_wifi_cwjap((char *)ack_info_buffer);
        return true;
    }
    return false;
}

bool _esp_parse_wifi_cwjap(char *str)
{
    str = strstr(str, "+CWJAP:");
    if (str == NULL)
    {
        return false;
    }
    if (sscanf(str, "+CWJAP:\"%63[^\"]\",\"%17[^\"]\",%d,%d", &esp_wifi_state.ssid, &esp_wifi_state.bssid, &esp_wifi_state.channel, &esp_wifi_state.rssi) != 4)
    {
        return true;
    }
    return false;
}

void DMA1_Stream6_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET)
    {
        BaseType_t woken = pdFALSE;
        xSemaphoreGiveFromISR(xEspUartTxSemaphore, &woken);
        portYIELD_FROM_ISR(woken);
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
    }
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART2);
        rb8_put(esp_rb, data);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        // ack接收完毕，按行处理缓冲区的内容
        if (!rb8_empty(esp_rb))
        {
            uint8_t i = 0;
            while (!rb8_empty(esp_rb) && i < sizeof(ack_info_buffer))
            {
                rb8_get(esp_rb, p_ack_info_buffer + i);
                i++;
                if (i >= 2 && p_ack_info_buffer[i - 2] == '\r' && p_ack_info_buffer[i - 1] == '\n')
                {
                    p_ack_info_buffer[i] = '\0';
                    esp_at_ack           = esp_match_from_ack((char *)(p_ack_info_buffer));
                    if (esp_at_ack != ESP_AT_ACK_NONE) // 只有当为有效ack时才通知send函数取
                    {
                        BaseType_t woken = pdFALSE;
                        xSemaphoreGiveFromISR(xEspUartRxSemaphore, &woken); // 通知send_cmd函数ack读取完毕
                        portYIELD_FROM_ISR(woken);
                    }
                    p_ack_info_buffer += i; // 移动到下一行的开始位置
                    i = 0;                  // 偏移量清零
                }
            }
        }
        volatile uint16_t dummy = USART_ReceiveData(USART2);
        USART_ClearITPendingBit(USART2, USART_IT_IDLE);
    }
}
