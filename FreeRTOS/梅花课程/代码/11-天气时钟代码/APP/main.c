#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>
#include <stdio.h>
#include "lvgl.h"

#define START_TASK_PRIO 4                    // 任务优先级
#define START_STK_SIZE  50                   // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define USART_TASK_PRIO 3             // 任务优先级（提高优先级，避免被GUI任务抢占）
#define USART_STK_SIZE  1024          // 任务堆栈大小
TaskHandle_t USARTTask_Handler;       // 任务句柄
void         usart_task(void *p_arg); // 任务函数

extern void gui_task_create(void);

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    usart_init(115200);
    timer_init();
    // LCD_Init(); // 移到gui_task中执行，避免在调度器启动前调用delay函数

    xTaskCreate(
        (TaskFunction_t)start_task,        // 任务函数
        (const char *)"start_task",        // 任务名称
        (uint16_t)START_STK_SIZE,          // 任务堆栈大小
        (void *)NULL,                      // 传递给任务函数的参数
        (UBaseType_t)START_TASK_PRIO,      // 任务优先级
        (TaskHandle_t *)&StartTask_Handler // 任务句柄
    );
    vTaskStartScheduler(); // 启动任务调度
}

void start_task(void *pvParameters)
{
    // 注册子任务
    xTaskCreate(
        (TaskFunction_t)usart_task,
        (const char *)"usart_task",
        (uint16_t)USART_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)USART_TASK_PRIO,
        (TaskHandle_t *)&USARTTask_Handler
    );
    gui_task_create();
    vTaskDelete(StartTask_Handler); // 删除开始任务
}

char ssid[]         = "HonorMagic6";
char pwd[]          = "22162216";
char weather_key[]  = "SWsbqmqZpeqLrKapw";
char weather_city[] = "ip";

void usart_task(void *p_arg)
{
    // WiFi初始化和连接
    esp_at_init();
    vTaskDelay(100);

    esp_wifi_init();
    vTaskDelay(100);

    esp_wifi_connected();
    vTaskDelay(100);

    printf("Start WiFi connect...\r\n");
    esp_at_wifi_connect(ssid, pwd);
    printf("WiFi connect finished\r\n");
    vTaskDelay(500); // WiFi连接可能需要较长时间

    if (esp_wifi_connected())
    {
        esp_send_weather_request(weather_key, weather_city, 5000);
        vTaskDelay(200);
    }

    // RTC和时间同步
    rtc_init();
    vTaskDelay(100);

    esp_sntp_init();
    vTaskDelay(100);

    esp_sntp_sync();
    vTaskDelay(100);

    rtc_print_time();

    while (1)
    {
        // if (xSemaphoreTake(xUsartRxSemaphore, portMAX_DELAY) == pdTRUE)
        // {
        //     if (!rb8_empty(rb))
        //     {
        //         uint32_t actual_size = rb8_actual_size(rb);
        //         rb8_gets(rb, data, actual_size);
        //         data[actual_size] = '\0';
        //         usart_send((char *)data);
        //     }
        // }
    }
}
