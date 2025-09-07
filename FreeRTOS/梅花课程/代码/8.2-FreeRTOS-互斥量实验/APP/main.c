#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "driver.h"
#include "data.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

// 低优先级任务
#define LOW_TASK_PRIO 2                  // 任务优先级
#define LOW_STK_SIZE  256                // 任务堆栈大小
TaskHandle_t LowPrioTask_Handler;        // 任务句柄
void         low_prio_task(void *p_arg); // 任务函数

// 高优先级任务
#define HIGH_TASK_PRIO 3                  // 任务优先级
#define HIGH_STK_SIZE  256                // 任务堆栈大小
TaskHandle_t HighPrioTask_Handler;        // 任务句柄
void         high_prio_task(void *p_arg); // 任务函数

SemaphoreHandle_t xUARTSemaphore; // 定义互斥量句柄

extern led_conf_t led0;

int main(void)
{
    usart_init(115200);
    led_init(&led0);

    // 创建互斥量
    xUARTSemaphore = xSemaphoreCreateMutex();

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
        (TaskFunction_t)high_prio_task,
        (const char *)"high_prio_task",
        (uint16_t)HIGH_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)HIGH_TASK_PRIO,
        (TaskHandle_t *)&HighPrioTask_Handler
    );
    xTaskCreate(
        (TaskFunction_t)low_prio_task,
        (const char *)"low_prio_task",
        (uint16_t)LOW_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)LOW_TASK_PRIO,
        (TaskHandle_t *)&LowPrioTask_Handler
    );

    vTaskDelete(StartTask_Handler); // 删除开始任务
}

void low_prio_task(void *p_arg)
{
    while (1)
    {
        // 获取互斥量
        if (xSemaphoreTake(xUARTSemaphore, portMAX_DELAY) == pdTRUE)
        {
            usart_send_str("[low] prio task running...printing Part A.\r\n");
            usart_send_str("[low] prio task running...printing Part B.\r\n");
            // 释放互斥量
            xSemaphoreGive(xUARTSemaphore);
        }
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void high_prio_task(void *p_arg)
{
    while (1)
    {
        // 获取互斥量
        if (xSemaphoreTake(xUARTSemaphore, portMAX_DELAY) == pdTRUE)
        {
            usart_send_str("<high> prio task running...printing Part A.\r\n");
            usart_send_str("<high> prio task running...printing Part B.\r\n");
            // 释放互斥量
            xSemaphoreGive(xUARTSemaphore);
        }
        // vTaskDelay(pdMS_TO_TICKS(700));
    }
}
