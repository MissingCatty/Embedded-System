#ifndef RTC_H
#define RTC_H

#include "stm32f4xx.h"

typedef struct
{
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  weekday;
} rtc_date_time_t;

void rtc_init(void);
void rtc_get_time(rtc_date_time_t *time);
void rtc_set_time(rtc_date_time_t *time);
void rtc_print_time(void);

#endif