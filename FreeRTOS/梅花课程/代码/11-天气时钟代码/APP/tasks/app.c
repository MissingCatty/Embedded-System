#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "my_tasks.h"
#include "driver.h"

#define SECOND(x) (x * 1000)
#define MINUTE(x) (x * 60 * 1000)

TimerHandle_t wifi_update_timer;    // 10s
TimerHandle_t sntp_sync_timer;      // 5s
TimerHandle_t dht11_update_timer;   // 2s
TimerHandle_t weather_update_timer; // 5min
TimerHandle_t time_update_timer;

typedef void (*function_t)(void);

extern void jobs_wifi_update(void);
extern void jobs_sntp_sync(void);
extern void jobs_weather_update(void);
extern void jobs_time_update(void);
extern void jobs_dht11_update(void);

// 包装将要放到队列的空参函数，如果不如此做，执行background_queue_put(function, NULL)会报错，因为function函数没有参数
void wrapper(void *func)
{
    function_t function = (function_t)func;
    function();
}

void timer_background_callback(TimerHandle_t xTimer)
{
    function_t function = (function_t)pvTimerGetTimerID(xTimer);
    background_queue_put(wrapper, function);
}

void timer_immediate_callback(TimerHandle_t xTimer)
{
    function_t function = (function_t)pvTimerGetTimerID(xTimer);
    function();
}

void app_start(void)
{
    background_queue_put(wrapper, esp_wifi_init); // 首先初始化wifi

    background_queue_put(wrapper, jobs_wifi_update);    // 获取wifi状态
    background_queue_put(wrapper, jobs_sntp_sync);      // 同步rtc
    background_queue_put(wrapper, jobs_time_update);    // 获取时间
    background_queue_put(wrapper, jobs_weather_update); // 获取天气信息
    background_queue_put(wrapper, jobs_dht11_update);   // 获取DHT11数据

    wifi_update_timer    = xTimerCreate("wifi_update_timer", pdMS_TO_TICKS(SECOND(2)), pdTRUE, jobs_wifi_update, timer_background_callback);
    sntp_sync_timer      = xTimerCreate("sntp_sync_timer", pdMS_TO_TICKS(MINUTE(1)), pdTRUE, jobs_sntp_sync, timer_background_callback);
    weather_update_timer = xTimerCreate("weather_update_timer", pdMS_TO_TICKS(MINUTE(30)), pdTRUE, jobs_weather_update, timer_background_callback);
    time_update_timer    = xTimerCreate("time_update_timer", pdMS_TO_TICKS(SECOND(1)), pdTRUE, jobs_time_update, timer_immediate_callback);
    dht11_update_timer   = xTimerCreate("dht11_update_timer", pdMS_TO_TICKS(SECOND(2)), pdTRUE, jobs_dht11_update, timer_background_callback);

    xTimerStart(wifi_update_timer, 0);
    xTimerStart(sntp_sync_timer, 0);
    xTimerStart(weather_update_timer, 0);
    xTimerStart(time_update_timer, 0);
    xTimerStart(dht11_update_timer, 0);
}
