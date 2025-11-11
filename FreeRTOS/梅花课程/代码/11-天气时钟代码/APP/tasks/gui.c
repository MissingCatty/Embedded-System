#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"

// 创建LVGL任务GUI_TASK
#define GUI_TASK_PRIO 2             // 任务优先级
#define GUI_STK_SIZE  1024          // 任务堆栈大小
TaskHandle_t GUI_Task_Handler;      // 任务句柄
void         gui_task(void *p_arg); // 任务函数

/**
 * @brief   在屏幕上绘制一个可指定圆角的实心矩形
 * @param   parent  父对象指针 (传入 NULL 则创建在当前活动屏幕上)
 * @param   x       矩形左上角的 X 坐标
 * @param   y       矩形左上角的 Y 坐标
 * @param   width   矩形的宽度
 * @param   height  矩形的高度
 * @param   color   矩形的颜色 (例如: 0xFF0000 表示红色)
 * @param   radius  圆角半径 (0 = 锐角, LV_RADIUS_CIRCLE = 圆形/胶囊形)
 * @return  lv_obj_t* 返回创建的矩形对象指针
 */
lv_obj_t *gui_draw_rectangle_radius(lv_obj_t *parent, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, int16_t radius)
{
    /* 1. 创建一个基础对象 (lv_obj) */
    lv_obj_t *rect = lv_obj_create(parent ? parent : lv_scr_act());

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

/**
 * @brief   创建一个文本标签
 * @param   parent  父对象指针 (传入 NULL 则创建在当前活动屏幕上)
 * @param   text    标签文本内容
 * @param   x       标签 X 坐标 (使用对齐时为 X 偏移量)
 * @param   y       标签 Y 坐标 (使用对齐时为 Y 偏移量)
 * @param   color   文字颜色 (例如: 0xFFFFFF 表示白色)
 * @param   font    字体指针 (例如: &lv_font_montserrat_16)
 * @param   align   对齐方式 (0: 使用绝对位置, LV_ALIGN_CENTER: 居中, LV_ALIGN_TOP_MID: 顶部居中等)
 * @return  lv_obj_t* 返回创建的标签对象指针
 */
lv_obj_t *gui_create_label(lv_obj_t *parent, const char *text, int16_t x, int16_t y, uint32_t color, const lv_font_t *font, lv_align_t align)
{
    /* 1. 创建标签对象 */
    lv_obj_t *label = lv_label_create(parent ? parent : lv_scr_act());

    /* 2. 设置文本内容 */
    lv_label_set_text(label, text);

    /* 3. 设置位置或对齐方式 */
    if (align == 0)
    {
        /* 使用绝对位置 */
        lv_obj_set_pos(label, x, y);
    } else
    {
        /* 使用对齐方式，x 和 y 为偏移量 */
        lv_obj_align(label, align, x, y);
    }

    /* 4. 设置文字颜色 */
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);

    /* 5. 设置字体 */
    if (font != NULL)
    {
        lv_obj_set_style_text_font(label, font, 0);
    }

    return label;
}

/**
 * @brief   在屏幕上显示图片
 * @param   parent  父对象指针 (传入 NULL 则创建在当前活动屏幕上)
 * @param   img_src 图片源 (可以是 C 数组指针，例如: &img_meihua)
 * @param   x       图片 X 坐标
 * @param   y       图片 Y 坐标
 * @param   width   图片宽度 (0 表示使用原始宽度)
 * @param   height  图片高度 (0 表示使用原始高度)
 * @return  lv_obj_t* 返回创建的图片对象指针
 */
lv_obj_t *gui_create_image(lv_obj_t *parent, const void *img_src, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    /* 1. 创建图片对象 */
    lv_obj_t *img = lv_image_create(parent ? parent : lv_scr_act());

    /* 2. 设置图片源 */
    lv_image_set_src(img, img_src);

    /* 3. 设置位置 */
    lv_obj_set_pos(img, x, y);

    /* 4. 设置大小 (如果指定了宽高) */
    if (width > 0 && height > 0)
    {
        lv_obj_set_size(img, width, height);
    }

    return img;
}

LV_IMG_DECLARE(weather_sun);
LV_IMG_DECLARE(wifi_on);
LV_IMG_DECLARE(wifi_off);
LV_IMG_DECLARE(thermometer);

void gui_task(void *p_arg)
{
    // LCD硬件初始化(必须在lvgl初始化之前)
    LCD_Init();

    // lvgl驱动初始化
    lvgl_ili9341_init(); // 初始化lvgl

    lv_obj_t *grey   = gui_draw_rectangle_radius(NULL, 10, 10, 220, 150, 0xA9A9A9, 6);
    lv_obj_t *blue   = gui_draw_rectangle_radius(NULL, 10, 170, 105, 140, 0x007BFF, 6);
    lv_obj_t *yellow = gui_draw_rectangle_radius(NULL, 125, 170, 105, 140, 0xFFA500, 6);

    lv_obj_t *time_hour   = gui_create_label(grey, "14", 40, 46, 0x000000, &lv_font_montserrat_48, 0);
    lv_obj_t *time_split  = gui_create_label(grey, ":", 100, 46, 0x000000, &lv_font_montserrat_48, 0);
    lv_obj_t *time_min    = gui_create_label(grey, "05", 120, 46, 0x000000, &lv_font_montserrat_48, 0);
    lv_obj_t *date        = gui_create_label(grey, "2025/11/11  Tuesday", 40, 120, 0x000000, &lv_font_montserrat_14, 0);
    lv_obj_t *wifi_state  = gui_create_image(grey, &wifi_off, 5, 5, 0, 0);
    lv_obj_t *wifi_device = gui_create_label(grey, "[Honor Magic6]", 80, 5, 0x000000, &lv_font_montserrat_14, 0);

    lv_obj_t *inner_env_title_bg = gui_draw_rectangle_radius(blue, 5, 5, 95, 20, 0xD4F2E7, 6);
    lv_obj_t *inner_env_title    = gui_create_label(inner_env_title_bg, "Inner", 0, 2, 0x000000, &lv_font_montserrat_14, LV_ALIGN_TOP_MID);
    lv_obj_t *temp_val           = gui_create_label(blue, "24", 30, 40, 0x000000, &lv_font_montserrat_40, 0);
    lv_obj_t *temp_unit          = gui_create_label(blue, "°C", 75, 30, 0x000000, &lv_font_montserrat_22, 0);
    lv_obj_t *humid_val          = gui_create_label(blue, "78", 0, -5, 0x000000, &lv_font_montserrat_40, LV_ALIGN_BOTTOM_MID);
    lv_obj_t *humid_unit         = gui_create_label(blue, "%", 35, -10, 0x000000, &lv_font_montserrat_22, LV_ALIGN_BOTTOM_MID);

    lv_obj_t *city_bg        = gui_draw_rectangle_radius(yellow, 5, 5, 95, 20, 0xFFE4B5, 6);
    lv_obj_t *city           = gui_create_label(city_bg, "NanJing", 0, 2, 0x000000, &lv_font_montserrat_14, LV_ALIGN_TOP_MID);
    lv_obj_t *city_temp      = gui_create_label(yellow, "25", 0, 30, 0x000000, &lv_font_montserrat_40, LV_ALIGN_TOP_MID);
    lv_obj_t *city_temp_unit = gui_create_label(yellow, "°C", 35, 25, 0x000000, &lv_font_montserrat_22, LV_ALIGN_TOP_MID);

    lv_obj_t *city_weather      = gui_create_image(yellow, &thermometer, 2, 75, 0, 0);
    lv_obj_t *city_weather_logo = gui_create_image(yellow, &weather_sun, 43, 75, 0, 0);
    LV_FONT_MONTSERRAT_16;
    lv_timer_handler();

    while (1)
    {
        vTaskDelay(1000); // 必须添加延时！空死循环会导致IDLE任务无法运行，引发系统问题
    }
}

void gui_task_create(void)
{
    xTaskCreate(
        (TaskFunction_t)gui_task,         // 任务函数
        (const char *)"gui_task",         // 任务名称
        (uint16_t)GUI_STK_SIZE,           // 任务堆栈大小
        (void *)NULL,                     // 传递给任务函数的参数
        (UBaseType_t)GUI_TASK_PRIO,       // 任务优先级
        (TaskHandle_t *)&GUI_Task_Handler // 任务句柄
    );
}