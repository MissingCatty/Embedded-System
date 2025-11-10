#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "driver.h"
#include <string.h>
#include <stdio.h>
#include "lvgl.h"

#define START_TASK_PRIO 4                    // 任务优先级
#define START_STK_SIZE  128                  // 任务堆栈大小
TaskHandle_t StartTask_Handler;              // 任务句柄
void         start_task(void *pvParameters); // 任务函数

#define USART_TASK_PRIO 1             // 任务优先级
#define USART_STK_SIZE  50            // 任务堆栈大小
TaskHandle_t USARTTask_Handler;       // 任务句柄
void         usart_task(void *p_arg); // 任务函数

// 创建LVGL任务GUI_TASK
#define GUI_TASK_PRIO 2             // 任务优先级
#define GUI_STK_SIZE  1024          // 任务堆栈大小
TaskHandle_t GUI_Task_Handler;      // 任务句柄
void         gui_task(void *p_arg); // 任务函数

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
    vTaskDelete(StartTask_Handler); // 删除开始任务
}

char ssid[]         = "HonorMagic6";
char pwd[]          = "22162216";
char weather_key[]  = "SWsbqmqZpeqLrKapw";
char weather_city[] = "ip";

void usart_task(void *p_arg)
{
    esp_at_init();
    esp_wifi_init();
    esp_wifi_connected();
    esp_at_wifi_connect(ssid, pwd);
    if (esp_wifi_connected())
    {
        esp_send_weather_request(weather_key, weather_city, 5000);
    }
    rtc_init();
    esp_sntp_init();
    esp_sntp_sync();
    rtc_print_time();

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

/**
 * @brief   在屏幕上绘制一个可指定圆角的实心矩形
 * @param   x       矩形左上角的 X 坐标
 * @param   y       矩形左上角的 Y 坐标
 * @param   width   矩形的宽度
 * @param   height  矩形的高度
 * @param   color   矩形的颜色 (例如: 0xFF0000 表示红色)
 * @param   radius  圆角半径 (0 = 锐角, LV_RADIUS_CIRCLE = 圆形/胶囊形)
 * @return  lv_obj_t* 返回创建的矩形对象指针
 */
lv_obj_t *gui_draw_rectangle_radius(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, int16_t radius)
{
    /* 1. 创建一个基础对象 (lv_obj) */
    lv_obj_t *rect = lv_obj_create(lv_scr_act());

    /* 2. 移除所有内边距 (padding) */
    lv_obj_set_style_pad_all(rect, 0, 0);

    /* 3. 设置指定位置 */
    lv_obj_set_pos(rect, x, y);

    /* 4. 设置指定大小 */
    lv_obj_set_size(rect, width, height);

    /* 5. 设置样式 */
    lv_obj_set_style_bg_color(rect, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(rect, 0, 0);

    /* 关键：设置指定的圆角半径 */
    lv_obj_set_style_radius(rect, radius, 0);

    return rect;
}

LV_IMG_DECLARE(weather_0);
void gui_task(void *p_arg)
{
    // lvgl驱动初始化
    lvgl_ili9341_init(); // 初始化lvgl

    //    lv_obj_t *screen = lv_screen_active();                                 // 获取当前为active的屏幕指针，一个屏幕为所有空间根容器，自己没有父容器
    //    lv_obj_set_style_bg_color(screen, lv_color_make(0xff, 0xff, 0xff), 0); // 将屏幕背景设置为白色
    //    lv_obj_t *label = lv_label_create(screen);                             // 在screen对象上创建一个label
    //    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);                           // 将label对象中心点对齐到屏幕的顶部中间，且无偏移
    //    lv_label_set_text(label, "Hello World!");
    //    lv_timer_handler();

    //		lv_obj_t * img = lv_image_create(lv_scr_act());
    //		lv_img_set_src(img, &img_meihua);
    //		lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    gui_draw_rectangle_radius(10, 10, 220, 150, 0xA9A9A9, 6);
    gui_draw_rectangle_radius(10, 170, 105, 140, 0x007BFF, 6);
    gui_draw_rectangle_radius(125, 170, 105, 140, 0xFFA500, 6);

    lv_timer_handler();
    while (1)
    {
    }
}
