#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>
#include <stdio.h>
#include "lvgl.h"
#include "my_tasks.h"

#define START_TASK_PRIO 4                    // 任务优先级
#define START_STK_SIZE  50                   // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    usart_init(115200);
    timer_init();
    esp_at_init();
    rtc_init();
    dht11_init();

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
    foreground_task_create();
    background_task_create();
    app_start();
    vTaskDelete(StartTask_Handler); // 删除开始任务
}

