#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include <string.h>

#define START_TASK_PRIO 4                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t Start_Task_Handler;             // 任务句柄
void         start_task(void *pvParameters); // 任务函数

// 创建LVGL任务GUI_TASK
#define GUI_TASK_PRIO 1             // 任务优先级
#define GUI_STK_SIZE  1024          // 任务堆栈大小
TaskHandle_t GUI_Task_Handler;      // 任务句柄
void         gui_task(void *p_arg); // 任务函数

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    usart_init(115200);
    delay_init();

    LCD_Init();

    xTaskCreate(
        (TaskFunction_t)start_task,         // 任务函数
        (const char *)"start_task",         // 任务名称
        (uint16_t)START_STK_SIZE,           // 任务堆栈大小
        (void *)NULL,                       // 传递给任务函数的参数
        (UBaseType_t)START_TASK_PRIO,       // 任务优先级
        (TaskHandle_t *)&Start_Task_Handler // 任务句柄
    );
    vTaskStartScheduler(); // 启动任务调度
}

void start_task(void *pvParameters)
{
    // 注册子任务
    xTaskCreate(
        (TaskFunction_t)gui_task,
        (const char *)"gui_task",
        (uint16_t)GUI_STK_SIZE,
        (void *)NULL,
        (UBaseType_t)GUI_TASK_PRIO,
        (TaskHandle_t *)&GUI_Task_Handler
    );
    vTaskDelete(Start_Task_Handler); // 删除开始任务
}

void gui_task(void *p_arg)
{
    // lvgl驱动初始化
    lvgl_ili9341_init(); // 初始化lvgl

    lv_obj_t *screen = lv_screen_active();                                 // 获取当前为active的屏幕指针，一个屏幕为所有空间根容器，自己没有父容器
    lv_obj_set_style_bg_color(screen, lv_color_make(0xff, 0xff, 0xff), 0); // 将屏幕背景设置为白色
    lv_obj_t *label = lv_label_create(screen);                             // 在screen对象上创建一个label
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);                           // 将label对象中心点对齐到屏幕的顶部中间，且无偏移
    lv_label_set_text(label, "Hello World!");
    lv_timer_handler();

    while (1)
    {
    }
}
