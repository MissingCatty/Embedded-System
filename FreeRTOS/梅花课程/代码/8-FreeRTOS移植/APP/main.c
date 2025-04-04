#include "stm32f4xx.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define LED0_TASK_PRIO 2             // 任务优先级
#define LED0_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t LED0Task_Handler;       // 任务句柄
void         led0_task(void *p_arg); // 任务函数

#define LED1_TASK_PRIO 3             // 任务优先级
#define LED1_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t LED1Task_Handler;       // 任务句柄
void         led1_task(void *p_arg); // 任务函数

#define FLOAT_TASK_PRIO 4             // 任务优先级
#define FLOAT_STK_SIZE  128           // 任务堆栈大小
TaskHandle_t FloatTask_Handler;       // 任务句柄
void         float_task(void *p_arg); // 任务函数

led_conf_t led0 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_5,
    GPIOE,
    Bit_RESET,
    Bit_SET
};

led_conf_t led1 = {
    RCC_AHB1Periph_GPIOE,
    GPIO_Pin_6,
    GPIOE,
    Bit_RESET,
    Bit_SET
};

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    led_init(&led0);
    led_init(&led1);
    delay_init();
    usart_init(115200);

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
        (TaskFunction_t)led0_task,
        (const char *)"led0_task",
        (uint16_t)LED0_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)LED0_TASK_PRIO,
        (TaskHandle_t *)&LED0Task_Handler
    );
    xTaskCreate(
        (TaskFunction_t)led1_task,
        (const char *)"led1_task",
        (uint16_t)LED1_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)LED1_TASK_PRIO,
        (TaskHandle_t *)&LED1Task_Handler
    );
    xTaskCreate(
        (TaskFunction_t)float_task,
        (const char *)"float_task",
        (uint16_t)FLOAT_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)FLOAT_TASK_PRIO,
        (TaskHandle_t *)&FloatTask_Handler
    );
    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void led0_task(void *p_arg)
{
    while (1)
    {
        led_on(&led0);
        vTaskDelay(500);
        led_off(&led0);
        vTaskDelay(500);
    }
}

void led1_task(void *p_arg)
{
    while (1)
    {
        led_on(&led1);
        vTaskDelay(1000);
        led_off(&led1);
        vTaskDelay(1000);
    }
}

void float_task(void *p_arg)
{
    static float temp = 0;
    while (1)
    {
        temp += 0.01f;
        char str[64];
        sprintf(str, "temp = %.2f\r\n", temp);
        usart_send_str(str);
        vTaskDelay(1000);
    }
}
