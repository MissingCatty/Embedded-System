#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "my_tasks.h"

#define SECOND(x) (x * 1000)
#define MINUTE(x) (x * 60 * 1000)

TimerHandle_t wifi_update_timer; // 10s
TimerHandle_t sntp_sync_timer;
TimerHandle_t dht11_update_timer;   // 2s
TimerHandle_t weather_update_timer; // 5min

typedef void (*function_t)(void);

extern void wifi_update(void);

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

void app_main(void)
{
    wifi_update_timer = xTimerCreate("wifi_update_timer", pdMS_TO_TICKS(SECOND(10)), pdTRUE, wifi_update, timer_background_callback);
    xTimerStart(wifi_update_timer, 0);
}
