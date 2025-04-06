#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>

#define START_TASK_PRIO 1                           // 任务优先级
#define START_STK_SIZE  128                         // 任务堆栈大小
static StaticTask_t StartTaskTCB;                   // 任务控制块
static StackType_t  StartTaskStk[START_STK_SIZE];   // 任务堆栈
TaskHandle_t        StartTask_Handler;              // 任务句柄
void                start_task(void *pvParameters); // 任务函数

#define TASK1_TASK_PRIO 2                         // 任务优先级
#define TASK1_STK_SIZE  128                       // 任务堆栈大小
static StaticTask_t Task1TaskTCB;                 // 任务控制块
static StackType_t  Task1TaskStk[TASK1_STK_SIZE]; // 任务堆栈
TaskHandle_t        TASK1Task_Handler;            // 任务句柄
void                task1_task(void *p_arg);      // 任务函数

#define TASK2_TASK_PRIO 2                         // 任务优先级
#define TASK2_STK_SIZE  128                       // 任务堆栈大小
static StaticTask_t Task2TaskTCB;                 // 任务控制块
static StackType_t  Task2TaskStk[TASK2_STK_SIZE]; // 任务堆栈
TaskHandle_t        TASK2Task_Handler;            // 任务句柄
void                task2_task(void *p_arg);      // 任务函数

extern led_conf_t led0;
extern led_conf_t led1;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    led_init(&led0);
    led_init(&led1);
    delay_init();
    usart_init(115200);
    LCD_Init();
    LCD_Display_Dir(1);

    StartTask_Handler = xTaskCreateStatic(
        (TaskFunction_t)start_task,
        (const char *)"start_task",
        (uint32_t)START_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)START_TASK_PRIO,
        (StackType_t *)&StartTaskStk,
        (StaticTask_t *)&StartTaskTCB
    );
    vTaskStartScheduler(); // 启动任务调度
}

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); // 进入临界区
    // 注册子任务
    TASK1Task_Handler = xTaskCreateStatic(
        (TaskFunction_t)task1_task,
        (const char *)"led0_task",
        (uint16_t)TASK1_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)TASK1_TASK_PRIO,
        (StackType_t *)&Task1TaskStk,
        (StaticTask_t *)&Task1TaskTCB
    );
    TASK2Task_Handler = xTaskCreateStatic(
        (TaskFunction_t)task2_task,
        (const char *)"led0_task",
        (uint16_t)TASK2_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)TASK2_TASK_PRIO,
        (StackType_t *)&Task2TaskStk,
        (StaticTask_t *)&Task2TaskTCB
    );
    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

u16 colors[] = {RED, YELLOW, BLUE};

void task1_task(void *p_arg)
{
    u16 sx = 20, sy = 20, width = 40;
    u16 ex = sx + width - 1, ey = sy + width - 1;
    u8  count = 0, task2_deleted = 0;
    while (1)
    {
        LCD_Fill(sx, sy, ex, ey, colors[count % 3]);
        led_on(&led0);
        vTaskDelay(500);
        led_off(&led0);
        vTaskDelay(500);
        count++;
        if (count == 5 && !task2_deleted)
        {
            vTaskDelete(TASK2Task_Handler); // 删除任务
            task2_deleted = 1;
        }
    }
}

void task2_task(void *p_arg)
{
    u16 sx = 200, sy = 20, width = 40;
    u16 ex = sx + width - 1, ey = sy + width - 1;
    u8  count = 0;
    while (1)
    {
        LCD_Fill(sx, sy, ex, ey, colors[count % 3]);
        led_on(&led1);
        vTaskDelay(500);
        led_off(&led1);
        vTaskDelay(500);
        count++;
    }
}
