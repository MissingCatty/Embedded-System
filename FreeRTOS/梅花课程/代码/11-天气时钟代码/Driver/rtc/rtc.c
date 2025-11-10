#include "rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

void rtc_init(void)
{
    // 步骤 1：开启 PWR 和 BKP 写保护
    /* Enable PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC domain */
    PWR_BackupAccessCmd(ENABLE);

    // 步骤 2：配置 RTC 时钟源（例如使用 LSE）
    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);

    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    RTC_WaitForSynchro();

    // 步骤3：配置 RTC_Init()
    RTC_InitTypeDef RTC_InitStructure;
    RTC_StructInit(&RTC_InitStructure);
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
    RTC_InitStructure.RTC_SynchPrediv  = 0xFF;
    RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);

    RTC_WaitForSynchro();
}

void _rtc_set_time_once(rtc_date_time_t *time)
{
    RTC_DateTypeDef RTC_DateStructure;
    RTC_TimeTypeDef RTC_TimeStructure;

    RTC_DateStructInit(&RTC_DateStructure);
    RTC_TimeStructInit(&RTC_TimeStructure);

    RTC_DateStructure.RTC_Year    = (uint8_t)(time->year - 2000);
    RTC_DateStructure.RTC_Month   = time->month;
    RTC_DateStructure.RTC_Date    = time->day;
    RTC_DateStructure.RTC_WeekDay = time->weekday;

    RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
    RTC_TimeStructure.RTC_Hours   = time->hour;
    RTC_TimeStructure.RTC_Minutes = time->minute;
    RTC_TimeStructure.RTC_Seconds = time->second;

    RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
}

void _rtc_get_time_once(rtc_date_time_t *time)
{
    RTC_DateTypeDef RTC_DateStructure;
    RTC_TimeTypeDef RTC_TimeStructure;

    // 确保结构体被正确初始化
    RTC_DateStructInit(&RTC_DateStructure);
    RTC_TimeStructInit(&RTC_TimeStructure);

    // 从硬件读取 BINARY 格式的数据
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

    time->year    = (uint16_t)RTC_DateStructure.RTC_Year + 2000;
    time->month   = RTC_DateStructure.RTC_Month;
    time->day     = RTC_DateStructure.RTC_Date;
    time->weekday = RTC_DateStructure.RTC_WeekDay;
    time->hour    = RTC_TimeStructure.RTC_Hours;
    time->minute  = RTC_TimeStructure.RTC_Minutes;
    time->second  = RTC_TimeStructure.RTC_Seconds;
}

/* 极端情况

如果 _rtc_get_time_once 在读取 time1 的中途发生时间跳变：

读取 time1.second (得到 59)。

RTC 跳变。

读取 time1.minute (得到 00)。

读取 time1.hour (得到 12)。

time1 的结果将是 12:00:59 —— 这是一个根本不存在的错误时间！

*/
void rtc_get_time(rtc_date_time_t *time)
{
    rtc_date_time_t time_tmp;
    do
    {
        // 保证两次读取的原子性
        taskENTER_CRITICAL();
        _rtc_get_time_once(&time_tmp);
        _rtc_get_time_once(time);
        taskEXIT_CRITICAL();
    } while (memcmp(&time_tmp, time, sizeof(rtc_date_time_t)) != 0);
}

/* 极端情况

_rtc_set_time_once在设置时，rtc的计时器即将自增，当设置完成后，立马计时器+1，那么设置的就不准
因为这相当于多设置一个计数值

*/
void rtc_set_time(rtc_date_time_t *time)
{
    rtc_date_time_t time_tmp;
    do
    {
        taskENTER_CRITICAL();
        _rtc_set_time_once(time);
        _rtc_get_time_once(&time_tmp);
        taskEXIT_CRITICAL();
    } while (time->second != time_tmp.second);
}

void rtc_print_time(void)
{ 
    rtc_date_time_t time;
    rtc_get_time(&time);
    printf("RTC: %02d:%02d:%02d %02d-%02d-%02d\n", time.hour, time.minute, time.second, time.day, time.month, time.year);
}