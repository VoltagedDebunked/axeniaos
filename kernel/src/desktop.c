#include "desktop.h"
#include "display.h"

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

void init_desktop(Desktop *desktop, WindowManager *wm) {
    desktop->wm = wm;
    strcpy(desktop->time, "12:00");
}

void draw_desktop_background(struct limine_framebuffer *fb) {
    // Draw background
    draw_rect(fb, 0, 0, fb->width, fb->height, COLOR_LIGHTGRAY);
    
    // Draw desktop icons
    const char* icon_names[] = {"My PC", "Documents", "Settings"};
    for (int i = 0; i < 3; i++) {
        uint32_t icon_y = 20 + (i * 70);
        draw_rect(fb, 20, icon_y, 40, 40, COLOR_GRAY);  // Icon box
        draw_string(fb, icon_names[i], 10, icon_y + 45, COLOR_BLACK);
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
    draw_all_windows(fb, desktop->wm);
    draw_taskbar(fb, desktop);
}