#include "rtc.h"
#include "lib/io.h"

void rtc_init() {
    // Disable interrupts
    outb(RTC_PORT, 0x8B);
    uint8_t prev = inb(RTC_DATA_PORT);
    outb(RTC_PORT, 0x8B);
    outb(RTC_DATA_PORT, prev | 0x40); // Enable periodic interrupts
}

void rtc_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
    outb(RTC_PORT, RTC_SECONDS);
    *seconds = inb(RTC_DATA_PORT);
    
    outb(RTC_PORT, RTC_MINUTES);
    *minutes = inb(RTC_DATA_PORT);
    
    outb(RTC_PORT, RTC_HOURS);
    *hours = inb(RTC_DATA_PORT);
}