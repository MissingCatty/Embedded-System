#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "my_tasks.h"
#include "driver.h"

typedef struct
{
    void (*func)(void *); // 工作函数
    void *param;          // 函数参数
} background_queue_msg_t;

static QueueHandle_t background_queue;

void background_queue_put(void (*func)(void *), void *param)
{
    if (background_queue)
    {
        background_queue_msg_t msg = {.func = func, .param = param};
        xQueueSend(background_queue, &msg, portMAX_DELAY);
    }
}

void background_task(void *vparm)
{
    background_queue_msg_t msg;
    while (1)
    {
        xQueueReceive(background_queue, &msg, portMAX_DELAY);
        msg.func(msg.param);
    }
}

void background_task_create(void)
{
    background_queue = xQueueCreate(1, sizeof(background_queue_msg_t));
    xTaskCreate((TaskFunction_t)background_task, "background_task", 1024, NULL, 3, NULL);
}
