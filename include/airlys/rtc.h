#ifndef AIRLYS_RTC_H
#define AIRLYS_RTC_H

#include <airlys/types.h>

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

/* Initialize CMOS RTC Driver */
void rtc_init(void);

/* Read current hardware real-time clock from CMOS */
rtc_time_t rtc_get_time(void);

#endif /* AIRLYS_RTC_H */
