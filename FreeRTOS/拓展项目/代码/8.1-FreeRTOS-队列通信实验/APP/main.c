#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "driver.h"
#include "data.h"
#include <stdio.h>
#include <string.h>

#define START_TASK_PRIO 1                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

// 生产者任务
#define SENDER_TASK_PRIO 2             // 任务优先级
#define SENDER_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t SenderTask_Handler;       // 任务句柄
void         sender_task(void *p_arg); // 任务函数

// 消费者任务
#define RECEIVER_TASK_PRIO 2             // 任务优先级
#define RECEIVER_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t ReceiverTask_Handler;       // 任务句柄
void         receiver_task(void *p_arg); // 任务函数

QueueHandle_t xDataQueue; // 定义共享队列句柄

extern led_conf_t led0;

int main(void)
{
    usart_init(115200);
    led_init(&led0);

    // 创建queue
    xDataQueue = xQueueCreate(5, sizeof(SensorData_t));
    if (xDataQueue == NULL)
    {
        usart_send_str("[Fatal Error]: queue creat failed, no enough memory!");
		return 1;
    }

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
        (TaskFunction_t)sender_task,
        (const char *)"sender_task",
        (uint16_t)SENDER_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)SENDER_TASK_PRIO,
        (TaskHandle_t *)&SenderTask_Handler
    );
    xTaskCreate(
        (TaskFunction_t)receiver_task,
        (const char *)"receiver_task",
        (uint16_t)RECEIVER_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)RECEIVER_TASK_PRIO,
        (TaskHandle_t *)&ReceiverTask_Handler
    );
    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void sender_task(void *p_arg)
{
    SensorData_t sensor_data;
    BaseType_t   xStatus;

    while (1)
    {
        random_sensor_data(&sensor_data);                                   // 生成随机数据s
        xStatus = xQueueSend(xDataQueue, &sensor_data, pdMS_TO_TICKS(100)); // 将数据发送到队列

        if (xStatus != pdPASS)
        {
            usart_send_str("[Fatal Error]: add queue error, queue full.\n");
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // 生产者每隔0.5s往队列里放一个元素，放的快
    }
}

void receiver_task(void *p_arg)
{
    SensorData_t sensor_data;
    BaseType_t   xStatus;

    while (1)
    {
        xStatus = xQueueReceive(xDataQueue, &sensor_data, pdMS_TO_TICKS(100));

        if (xStatus == pdPASS)
        {
            char msg[100];
            sprintf(msg, "[Receiver]: receive sensor data [id: %u], [val: %u], [timestamp: %u].\n", sensor_data.id, sensor_data.val, sensor_data.timestamp);
            usart_send_str(msg);
            // 队列可取，则led0亮
            led_on(&led0);
        } else
        {
            // 队列为空，则led0灭
            led_off(&led0);
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // 消费者每隔2s才取一次，取的比放的慢，会造成队列满的情况
    }
}
