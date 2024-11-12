#include "desktop.h"
#include "display.h"
#include "rtc.h"

char *strcpy(char *dest, const char *src) {
    char *original_dest = dest; // Save the original pointer to the destination

    while (*src != '\0') { // Loop until the end of the source string
        *dest = *src;      // Copy each character
        dest++;            // Move to the next character in the destination
        src++;             // Move to the next character in the source
    }
    *dest = '\0'; // Null-terminate the destination string

    return original_dest; // Return the original destination pointer
}

void init_desktop(Desktop *desktop, WindowManager *wm) {
    desktop->wm = wm;

    // Get the current time from the RTC
    uint8_t hours, minutes, seconds;
    rtc_get_time(&hours, &minutes, &seconds);

    // Manually format the time as "HH:MM"
    desktop->time[0] = '0' + (hours / 10); // Tens place of hours
    desktop->time[1] = '0' + (hours % 10); // Units place of hours
    desktop->time[2] = ':';                 // Separator
    desktop->time[3] = '0' + (minutes / 10); // Tens place of minutes
    desktop->time[4] = '0' + (minutes % 10); // Units place of minutes
    desktop->time[5] = '\0';                // Null-terminate the string
}

void draw_desktop_background(struct limine_framebuffer *fb) {
    // Draw background
    draw_rect(fb, 0, 0, fb->width, fb->height, COLOR_BLUE);
    
    // Draw desktop icons
    const char* icon_names[] = {"My PC", "Documents", "Settings"};
    for (int i = 0; i < 3; i++) {
        uint32_t icon_y = 20 + (i * 70);
        draw_rect(fb, 20, icon_y, 40, 40, COLOR_GRAY);  // Icon box
        draw_string(fb, icon_names[i], 10, icon_y + 45, COLOR_WHITE);
    }
}

void draw_taskbar(struct limine_framebuffer *fb, Desktop *desktop) {
    // Draw main taskbar
    draw_rect(fb, 0, fb->height - TASKBAR_HEIGHT, fb->width, TASKBAR_HEIGHT, COLOR_DARKGRAY);
    
    // Draw start button
    draw_rect(fb, 0, fb->height - TASKBAR_HEIGHT, BUTTON_WIDTH, TASKBAR_HEIGHT, COLOR_GRAY);
    draw_string(fb, "Start", 10, fb->height - 15, COLOR_WHITE);
    
    // Draw time
    draw_string(fb, desktop->time, fb->width - 50, fb->height - 15, COLOR_WHITE);
}

void draw_desktop(struct limine_framebuffer *fb, Desktop *desktop) {
    draw_desktop_background(fb);
    draw_taskbar(fb, desktop);
}