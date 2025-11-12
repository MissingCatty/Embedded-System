#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "driver.h"
#include "my_tasks.h"
#include "queue.h"
#include "string.h"

LV_IMG_DECLARE(weather_sun);
LV_IMG_DECLARE(weather_cloud);
LV_IMG_DECLARE(weather_nosun);
LV_IMG_DECLARE(weather_rain);
LV_IMG_DECLARE(weather_thounderrain);
LV_IMG_DECLARE(weather_snow);
LV_IMG_DECLARE(weather_na);
LV_IMG_DECLARE(wifi_on);
LV_IMG_DECLARE(wifi_off);

TaskHandle_t  GUI_Task_Handler;
QueueHandle_t foreground_queue;

lv_obj_t *time_hour;
lv_obj_t *time_split;
lv_obj_t *time_min;
lv_obj_t *date;
lv_obj_t *wifi_state;
lv_obj_t *wifi_device;

lv_obj_t *temp_val;
lv_obj_t *humid_val;

lv_obj_t *city;
lv_obj_t *city_temp;
lv_obj_t *city_weather_logo;

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
lv_obj_t *foreground_draw_rectangle_radius(lv_obj_t *parent, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, int16_t radius)
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
lv_obj_t *foreground_create_label(lv_obj_t *parent, const char *text, int16_t x, int16_t y, uint32_t color, const lv_font_t *font, lv_align_t align)
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
lv_obj_t *foreground_create_image(lv_obj_t *parent, const void *img_src, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
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

void foreground_update_wifi_state(foreground_queue_msg_t *msg)
{
    bool connected = msg->data.wifi_state.connected;
    if (connected)
    {
        char wifi_ssid[64];
        sprintf(wifi_ssid, "[%s]", msg->data.wifi_state.ssid);
        lv_label_set_text(wifi_device, wifi_ssid);
        lv_image_set_src(wifi_state, &wifi_on);
    } else
    {
        lv_label_set_text(wifi_device, "[Wait to connect]");
        lv_image_set_src(wifi_state, &wifi_off);
    }
}

void foreground_update_weather(foreground_queue_msg_t *msg)
{
    lv_label_set_text(city, msg->data.weather.city);
    char temp[10];
    sprintf(temp, "%d", msg->data.weather.temp);
    lv_label_set_text(city_temp, temp);
    const lv_image_dsc_t *src;
    switch (msg->data.weather.weather_code)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        src = &weather_sun;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        src = &weather_cloud;
        break;
    case 9:
        src = &weather_nosun;
        break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
        src = &weather_rain;
        break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
        src = &weather_snow;
        break;
    default:
        src = &weather_na;
        break;
    }
    lv_image_set_src(city_weather_logo, src);
}

void foreground_update_time(foreground_queue_msg_t *msg)
{
    char date_str[32];
    char weekday[10];
    switch (msg->data.time.weekday)
    {
    case 1:
        strcpy(weekday, "Monday");
        break;
    case 2:
        strcpy(weekday, "Tuesday");
        break;
    case 3:
        strcpy(weekday, "Wednesday");
        break;
    case 4:
        strcpy(weekday, "Thursday");
        break;
    case 5:
        strcpy(weekday, "Friday");
        break;
    case 6:
        strcpy(weekday, "Saturday");
        break;
    case 7:
        strcpy(weekday, "Sunday");
        break;
    default:
        strcpy(weekday, "Weekday");
    }
    sprintf(date_str, "%04d/%02d/%02d  %s", msg->data.time.year, msg->data.time.month, msg->data.time.day, weekday);
    char hour_str[3], minute_str[3];
    sprintf(hour_str, "%02d", msg->data.time.hour);
    sprintf(minute_str, "%02d", msg->data.time.minute);
    lv_label_set_text(date, date_str);
    lv_label_set_text(time_hour, hour_str);
    lv_label_set_text(time_min, minute_str);
}

void foreground_update_dht11(foreground_queue_msg_t *msg)
{
    char temp_str[5], humid_str[5];
    sprintf(temp_str, "%d", msg->data.dht11.temperature);
    sprintf(humid_str, "%d", msg->data.dht11.humidity);
    lv_label_set_text(temp_val, temp_str);
    lv_label_set_text(humid_val, humid_str);
}

void foreground_operation(foreground_queue_msg_t *msg)
{
    switch (msg->op)
    {
    case FOREGROUND_OP_WIFI_STATE:
        foreground_update_wifi_state(msg);
        break;
    case FOREGROUND_OP_WEATHER:
        foreground_update_weather(msg);
        break;
    case FOREGROUND_OP_TIME_UPDATE:
        foreground_update_time(msg);
        break;
    case FOREGROUND_OP_DHT11_UPDATE:
        foreground_update_dht11(msg);
        break;
    default:
        break;
    }
}

void foreground_queue_put(foreground_queue_msg_t *msg)
{
    if (foreground_queue)
        xQueueSend(foreground_queue, msg, portMAX_DELAY);
}

void foreground_queue_create(void)
{
    foreground_queue = xQueueCreate(16, sizeof(foreground_queue_msg_t));
}

void foreground_task(void *p_arg)
{
    // LCD硬件初始化(必须在lvgl初始化之前)
    LCD_Init();

    // lvgl驱动初始化
    lvgl_ili9341_init(); // 初始化lvgl

    // 创建队列
    foreground_queue_create();

    lv_obj_t *grey   = foreground_draw_rectangle_radius(NULL, 10, 10, 220, 150, 0xA9A9A9, 6);
    lv_obj_t *blue   = foreground_draw_rectangle_radius(NULL, 10, 170, 105, 140, 0x007BFF, 6);
    lv_obj_t *yellow = foreground_draw_rectangle_radius(NULL, 125, 170, 105, 140, 0xFFA500, 6);

    time_hour   = foreground_create_label(grey, "--", 40, 46, 0x000000, &lv_font_montserrat_48, 0);
    time_split  = foreground_create_label(grey, ":", 100, 46, 0x000000, &lv_font_montserrat_48, 0);
    time_min    = foreground_create_label(grey, "--", 130, 46, 0x000000, &lv_font_montserrat_48, 0);
    date        = foreground_create_label(grey, "----/--/--  Weekday", 40, 120, 0x000000, &lv_font_montserrat_14, 0);
    wifi_state  = foreground_create_image(grey, &wifi_off, 5, 5, 0, 0);
    wifi_device = foreground_create_label(grey, "[Wait to connect]", 80, 5, 0x000000, &lv_font_montserrat_14, 0);

    lv_obj_t *inner_env_title_bg = foreground_draw_rectangle_radius(blue, 5, 5, 95, 20, 0xD4F2E7, 6);
    lv_obj_t *inner_env_title    = foreground_create_label(inner_env_title_bg, "Inner", 0, 2, 0x000000, &lv_font_montserrat_14, LV_ALIGN_TOP_MID);
    temp_val                     = foreground_create_label(blue, "--", 30, 40, 0x000000, &lv_font_montserrat_40, 0);
    lv_obj_t *temp_unit          = foreground_create_label(blue, "°C", 75, 30, 0x000000, &lv_font_montserrat_22, 0);
    humid_val                    = foreground_create_label(blue, "--", 0, -5, 0x000000, &lv_font_montserrat_40, LV_ALIGN_BOTTOM_MID);
    lv_obj_t *humid_unit         = foreground_create_label(blue, "%", 35, -10, 0x000000, &lv_font_montserrat_22, LV_ALIGN_BOTTOM_MID);

    lv_obj_t *city_bg        = foreground_draw_rectangle_radius(yellow, 5, 5, 95, 20, 0xFFE4B5, 6);
    city                     = foreground_create_label(city_bg, "City", 0, 2, 0x000000, &lv_font_montserrat_14, LV_ALIGN_TOP_MID);
    city_temp                = foreground_create_label(yellow, "--", -10, 35, 0x000000, &lv_font_montserrat_40, LV_ALIGN_TOP_MID);
    lv_obj_t *city_temp_unit = foreground_create_label(yellow, "°C", 25, 35, 0x000000, &lv_font_montserrat_22, LV_ALIGN_TOP_MID);

    city_weather_logo = foreground_create_image(yellow, &weather_na, 20, 80, 0, 0);
    lv_timer_handler();

    while (1)
    {
        foreground_queue_msg_t msg;
        if (xQueueReceive(foreground_queue, &msg, pdMS_TO_TICKS(10)) == pdTRUE)
            foreground_operation(&msg);
        lv_timer_handler();
    }
}

void foreground_task_create(void)
{
    xTaskCreate((TaskFunction_t)foreground_task, "foreground_task", 1024, NULL, 2, &GUI_Task_Handler);
}