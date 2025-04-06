#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define TASK1_TASK_PRIO 2             // 任务优先级
#define TASK1_STK_SIZE  128           // 任务堆栈大小
TaskHandle_t TASK1Task_Handler;       // 任务句柄
void         task1_task(void *p_arg); // 任务函数

#define TASK2_TASK_PRIO 3             // 任务优先级
#define TASK2_STK_SIZE  128           // 任务堆栈大小
TaskHandle_t TASK2Task_Handler;       // 任务句柄
void         task2_task(void *p_arg); // 任务函数

#define KEY_TASK_PRIO 1             // 任务优先级
#define KEY_STK_SIZE  128           // 任务堆栈大小
TaskHandle_t KEYTask_Handler;       // 任务句柄
void         key_task(void *p_arg); // 任务函数

extern led_conf_t led0;
extern led_conf_t led1;
extern key_conf_t key3;
extern key_conf_t key4;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    led_init(&led0);
    led_init(&led1);
    key_init(&key3);
    key_init(&key4);
    delay_init();
    usart_init(115200);
    LCD_Init();
    LCD_Display_Dir(1);

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
    // 注册子任务
    xTaskCreate(
        (TaskFunction_t)task1_task,
        (const char *)"task1_task",
        (uint16_t)TASK1_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)TASK1_TASK_PRIO,
        (TaskHandle_t *)&TASK1Task_Handler
    );
    xTaskCreate(
        (TaskFunction_t)task2_task,
        (const char *)"task2_task",
        (uint16_t)TASK2_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)TASK2_TASK_PRIO,
        (TaskHandle_t *)&TASK2Task_Handler
    );
    xTaskCreate(
        (TaskFunction_t)key_task,
        (const char *)"key_task",
        (uint16_t)KEY_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)KEY_TASK_PRIO,
        (TaskHandle_t *)&KEYTask_Handler
    );
    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void task1_task(void *p_arg)
{
    LCD_ShowString(0, 10, 120, 16, 16, "Task1 has run: ");
    uint16_t count = 0;
    char     str[20];
    while (1)
    {
        led_on(&led1);
        vTaskDelay(500);
        led_off(&led1);
        vTaskDelay(500);
        sprintf(str, "%d s.", ++count);
        LCD_ShowString(130, 10, 150, 16, 16, (uint8_t *)str);
    }
}

void task2_task(void *p_arg)
{
    LCD_ShowString(0, 60, 120, 16, 16, "Task2 has run: ");
    uint16_t count = 0;
    char     str[20];
    while (1)
    {
        led_on(&led0);
        vTaskDelay(500);
        led_off(&led0);
        vTaskDelay(500);
        sprintf(str, "%d s.", ++count);
        LCD_ShowString(130, 60, 150, 16, 16, (uint8_t *)str);
    }
}

void key_task(void *p_arg)
{
    while (1)
    {
        // 轮询key3状态
        if (key_pushed(&key3))
        {
            // 延时消抖
            delay_ms(10);
            if (key_pushed(&key3))
            {
                // 挂起task1
                vTaskSuspend(TASK1Task_Handler);
                LCD_Fill(280, 10, 280 + 20, 10 + 20, RED);
            }
        }
    }
}

void EXTI0_IRQHandler(void)
{
    // key4中断函数
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        delay_ms(10);
        if (key_pushed(&key4))
        {
            // 恢复task1
            xTaskResumeFromISR(TASK1Task_Handler);
            LCD_Fill(280, 10, 280 + 20, 10 + 20, GREEN);
        }
        EXTI_ClearITPendingBit(EXTI_Line0); // 清除中断标志位
    }
}

void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}
