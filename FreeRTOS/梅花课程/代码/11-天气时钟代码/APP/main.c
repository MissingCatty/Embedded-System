#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "driver.h"
#include <string.h>
#include <stdio.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define USART_TASK_PRIO 3             // 任务优先级
#define USART_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t USARTTask_Handler;       // 任务句柄
void         usart_task(void *p_arg); // 任务函数

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    usart_init(115200);
    timer_init();

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
    // dht11_task_create();
    vTaskDelete(StartTask_Handler); // 删除开始任务
}

void usart_task(void *p_arg)
{
    esp_at_init();
    esp_wifi_init();
    esp_wifi_query_state(&esp_wifi_state);
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
