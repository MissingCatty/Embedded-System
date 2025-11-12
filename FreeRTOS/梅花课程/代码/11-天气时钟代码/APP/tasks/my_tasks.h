#ifndef MY_TASKS_H
#define MY_TASKS_H

#include "lvgl.h"
#include "stm32f4xx.h"

typedef enum {
    FOREGROUND_OP_NONE,
    FOREGROUND_OP_WIFI_STATE,
    FOREGROUND_OP_WEATHER,
    FOREGROUND_OP_TIME_UPDATE,
    FOREGROUND_OP_DHT11_UPDATE
} foreground_operation_t;

typedef struct
{
    foreground_operation_t op;
    union {
        struct
        {
            char ssid[64];
            bool connected;
        } wifi_state;

        struct
        {
            uint8_t temp;
            uint8_t weather_code;
            char    city[32];
        } weather;

        struct
        {
            uint16_t year;
            uint8_t  month;
            uint8_t  day;
            uint8_t  hour;
            uint8_t  minute;
            uint8_t  second;
            uint8_t  weekday;
        } time;

        struct
        {
            uint8_t humidity;
            uint8_t temperature;
        } dht11;
    } data;
} foreground_queue_msg_t;

void foreground_task_create(void);
void foreground_queue_put(foreground_queue_msg_t *msg);

void background_task_create(void);
void background_queue_put(void (*func)(void *), void *param);

void app_start(void);

#endif