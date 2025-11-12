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

#define esp_time_t         rtc_date_time_t

#define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))

#define ESP_RX_BUFFER_SIZE 1 * 1024
uint8_t       esp_rx_buffer[ESP_RX_BUFFER_SIZE];
ringbuffer8_t esp_rb;

static SemaphoreHandle_t xEspUartTxSemaphore;
static SemaphoreHandle_t xEspUartRxSemaphore;

esp_at_ack_t esp_at_ack;

uint8_t  ack_info_buffer[1024];               // 信息接收缓冲区
uint8_t *p_ack_info_buffer = ack_info_buffer; // 接收缓冲区起始指针

esp_wifi_state_t esp_wifi_state;

esp_weather_info_t weather_info;

esp_time_t esp_sntp_time;

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
    uint32_t len = strlen(str);

    // 先禁用DMA，确保配置时DMA处于停止状态
    DMA_Cmd(DMA1_Stream6, DISABLE);

    // 等待DMA真正禁用（关键：避免配置冲突）
    while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE);

    // 配置DMA传输参数
    DMA1_Stream6->M0AR = (uint32_t)str;
    DMA1_Stream6->NDTR = len;

    // 清除传输完成标志
    DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);

    // 使能DMA开始传输
    DMA_Cmd(DMA1_Stream6, ENABLE);

    // 等待DMA传输完成（通过中断释放信号量）
    xSemaphoreTake(xEspUartTxSemaphore, portMAX_DELAY);

    USART_ClearFlag(USART2, USART_FLAG_TC);
}

esp_at_ack_t esp_at_send_cmd(const char cmd[], uint16_t timeout)
{
    p_ack_info_buffer = ack_info_buffer; // 发送指令之前先把返回信息的指针复原
    esp_at_send(cmd);

    if (xSemaphoreTake(xEspUartRxSemaphore, pdMS_TO_TICKS(timeout)) == pdTRUE)
    {
        return esp_at_ack;
    }
    return ESP_AT_ACK_NONE;
}

bool esp_wifi_init(void)
{
    return esp_at_send_cmd("AT+CWMODE=1\r\n", 2000) == ESP_AT_ACK_OK;
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
    if (esp_at_send_cmd(cmd, 2000) == ESP_AT_ACK_OK)
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

bool _esp_parse_weather_info(char *response)
{
    response = strstr(response, "\"results\":");
    if (!response)
    {
        return false; // 根result找不到
    }

    char *location_response = strstr(response, "\"location\":");
    if (!location_response)
    {
        return false; // location域找不到
    }

    char *name_response = strstr(location_response, "\"name\":");
    if (name_response)
    {
        sscanf(name_response, "\"name\":\"%32[^\"]\"", weather_info.city);
    }

    char *now_response = strstr(location_response, "\"now\":");
    if (!now_response)
    {
        return false; // now域找不到
    }

    char *now_text_response = strstr(now_response, "\"text\":");
    if (now_text_response)
    {
        sscanf(now_text_response, "\"text\":\"%32[^\"]\"", weather_info.weather);
    }

    char *now_code_response = strstr(now_response, "\"code\":");
    if (now_code_response)
    {
        sscanf(now_code_response, "\"code\":\"%d\"", &weather_info.weather_code);
    }

    char *now_temperature_response = strstr(now_response, "\"temperature\":");
    if (now_temperature_response)
    {
        sscanf(now_temperature_response, "\"temperature\":\"%f\"", &weather_info.temperature);
    }

    return true;
}

bool esp_send_weather_request(char key[], char location[], uint16_t timeout)
{
    char request[256], url[256];
    sprintf(url, "https://api.seniverse.com/v3/weather/now.json?key=%s&location=%s&language=en&unit=c", key, location);
    sprintf(request, "AT+HTTPCLIENT=2,1,\"%s\",,,2\r\n", url);
    if (esp_at_send_cmd(request, timeout) == ESP_AT_ACK_OK)
    {
        // 解析返回信息
        if (!_esp_parse_weather_info((char *)ack_info_buffer))
        {
            printf("weather parse error.\n");
            return false;
        }
        return true;
    }
    return false;
}

bool esp_sntp_init(void)
{
    if (esp_at_send_cmd("AT+CIPSNTPCFG=1,8\r\n", 2000) == ESP_AT_ACK_OK)
    {
        return true;
    }
    return false;
}

void _convert_weekday_from_str_to_num(char weekday[])
{
    if (!strcmp(weekday, "Mon"))
    {
        esp_sntp_time.weekday = 1;
    } else if (!strcmp(weekday, "Tue"))
    {
        esp_sntp_time.weekday = 2;
    } else if (!strcmp(weekday, "Wed"))
    {
        esp_sntp_time.weekday = 3;
    } else if (!strcmp(weekday, "Thu"))
    {
        esp_sntp_time.weekday = 4;
    } else if (!strcmp(weekday, "Fri"))
    {
        esp_sntp_time.weekday = 5;
    } else if (!strcmp(weekday, "Sat"))
    {
        esp_sntp_time.weekday = 6;
    } else if (!strcmp(weekday, "Sun"))
    {
        esp_sntp_time.weekday = 7;
    }
}

void _convert_month_from_str_to_num(char month[])
{
    if (!strcmp(month, "Jan"))
    {
        esp_sntp_time.month = 1;
    } else if (!strcmp(month, "Feb"))
    {
        esp_sntp_time.month = 2;
    } else if (!strcmp(month, "Mar"))
    {
        esp_sntp_time.month = 3;
    } else if (!strcmp(month, "Apr"))
    {
        esp_sntp_time.month = 4;
    } else if (!strcmp(month, "May"))
    {
        esp_sntp_time.month = 5;
    } else if (!strcmp(month, "Jun"))
    {
        esp_sntp_time.month = 6;
    } else if (!strcmp(month, "Jul"))
    {
        esp_sntp_time.month = 7;
    } else if (!strcmp(month, "Aug"))
    {
        esp_sntp_time.month = 8;
    } else if (!strcmp(month, "Sep"))
    {
        esp_sntp_time.month = 9;
    } else if (!strcmp(month, "Oct"))
    {
        esp_sntp_time.month = 10;
    } else if (!strcmp(month, "Nov"))
    {
        esp_sntp_time.month = 11;
    } else if (!strcmp(month, "Dec"))
    {
        esp_sntp_time.month = 12;
    }
}

// +CIPSNTPTIME:Tue Oct 19 17:47:56 2021
bool _parse_sntp_time(char *response)
{
    response = strstr(response, "+CIPSNTPTIME:");
    if (response)
    {
        char weekday[4] = {0}, month[4] = {0};
        sscanf(response, "+CIPSNTPTIME:%3s %3s %hhu %hhu:%hhu:%hhu %hu", weekday, month, &esp_sntp_time.day, &esp_sntp_time.hour, &esp_sntp_time.minute, &esp_sntp_time.second, &esp_sntp_time.year);
        _convert_weekday_from_str_to_num(weekday);
        _convert_month_from_str_to_num(month);
        return true;
    }
    return false;
}

bool esp_sntp_sync(void)
{
    if (esp_at_send_cmd("AT+CIPSNTPTIME?\r\n", 2000) == ESP_AT_ACK_OK)
    {
        _parse_sntp_time((char *)ack_info_buffer);
        rtc_set_time(&esp_sntp_time);
        return true;
    }
    return false;
}

// SWsbqmqZpeqLrKapw
// https://api.seniverse.com/v3/weather/now.json?key=SWsbqmqZpeqLrKapw&location=ip&language=en&unit=c
// {"results":[{"location":{"id":"WTSQQYHVQ973","name":"南京","country":"CN","path":"南京,南京,江苏,中国","timezone":"Asia/Shanghai","timezone_offset":"+08:00"},"now":{"text":"小雨","code":"13","temperature":"16"},"last_update":"2025-11-07T16:27:28+08:00"}]}
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
            uint16_t i = 0;
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
