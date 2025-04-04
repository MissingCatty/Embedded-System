#include "stm32f4xx.h"
#include "main.h"
#include <string.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  256                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define INTERRUPT_TASK_PRIO 2             // 任务优先级
#define INTERRUPT_STK_SIZE  256           // 任务堆栈大小
TaskHandle_t INTERRUPTTask_Handler;       // 任务句柄
void         interrupt_task(void *p_arg); // 任务函数

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();
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
    taskENTER_CRITICAL(); // 进入临界区

    // 注册中断测试任务
    xTaskCreate(
        (TaskFunction_t)interrupt_task,
        (const char *)"interrupt_task",
        (uint16_t)INTERRUPT_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)INTERRUPT_TASK_PRIO,
        (TaskHandle_t *)&INTERRUPTTask_Handler
    );

    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void interrupt_task(void *p_arg)
{
    while (1)
    {
        // 中断关闭五秒
        portDISABLE_INTERRUPTS();
        delay_s(5);
        // 中断开启五秒
        portENABLE_INTERRUPTS();
        delay_s(5);
    }
}
