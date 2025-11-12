#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "driver.h"
#include "my_tasks.h"
#include "string.h"

void jobs_wifi_update(void)
{
    bool  connected = esp_wifi_connected();
    char *ssid;
    if (connected)
    {
        ssid = esp_wifi_state.ssid;
    } else if (esp_at_wifi_connect("HonorMagic6", "22162216"))
    {
        ssid      = esp_wifi_state.ssid;
        connected = true;
        printf("[jobs_wifi_update]: wifi connected\n");
    } else
    {
        printf("[jobs_wifi_update]: connect failed\n");
    }

    // 向前台队列发送数据
    foreground_queue_msg_t msg;
    msg.op                        = FOREGROUND_OP_WIFI_STATE;
    msg.data.wifi_state.connected = connected;
    strcpy(msg.data.wifi_state.ssid, ssid);
    foreground_queue_put(&msg);
}

void jobs_sntp_sync(void)
{
    if (esp_sntp_sync())
    {
        printf("[jobs_sntp_snyc]: sntp sync success\n");
    } else
    {
        printf("[jobs_sntp_snyc]: sntp sync failed\n");
    }
}

void jobs_weather_update(void)
{
    if (esp_wifi_connected())
    {
        esp_send_weather_request("SWsbqmqZpeqLrKapw", "ip", 5000);
        foreground_queue_msg_t msg;
        msg.op                        = FOREGROUND_OP_WEATHER;
        msg.data.weather.temp         = weather_info.temperature;
        msg.data.weather.weather_code = weather_info.weather_code;
        strcpy(msg.data.weather.city, (char *)weather_info.city);
        foreground_queue_put(&msg);
        printf("[jobs_weather_update]: weather updated\n");
    } else
    {
        printf("[jobs_weather_update]: wifi not connected\n");
    }
}

void jobs_time_update(void)
{
    foreground_queue_msg_t msg;
    msg.op = FOREGROUND_OP_TIME_UPDATE;
    rtc_date_time_t time;
    rtc_get_time(&time);
    memcpy(&msg.data.time, &time, sizeof(rtc_date_time_t));
    foreground_queue_put(&msg);
}

void jobs_dht11_update(void)
{
    foreground_queue_msg_t msg;
    msg.op = FOREGROUND_OP_DHT11_UPDATE;
    dht11_receive_data();
    msg.data.dht11.temperature = dht11_temp_integer;
    msg.data.dht11.humidity    = dht11_humid_integer;
    foreground_queue_put(&msg);
}