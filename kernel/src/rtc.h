#ifndef RTC_H
#define RTC_H

#include <stdint.h>

#define RTC_PORT 0x70
#define RTC_DATA_PORT 0x71

// RTC registers
#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS   0x04

void rtc_init();
void rtc_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);

#endif // RTC_H