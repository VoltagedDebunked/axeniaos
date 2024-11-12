#include "window.h"
#include "display.h"
#include <stdint.h>
#include <stdbool.h>

typedef long size_t;

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    // Pad remaining bytes with nulls if any
    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

void init_window_manager(WindowManager *wm) {
    wm->window_count = 0;
    wm->active_window = -1;
}

int create_window(WindowManager *wm, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const char *title) {
    if (wm->window_count >= MAX_WINDOWS) {
        return -1;
    }

    Window *window = &wm->windows[wm->window_count];
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    strncpy(window->title, title, 63);
    window->title[63] = '\0';
    window->is_visible = true;
    window->is_active = true;

    if (wm->active_window == -1) {
        wm->active_window = wm->window_count;
    }

    return wm->window_count++;
}

void draw_window(struct limine_framebuffer *fb, Window *window) {
    if (!window->is_visible) {
        return;
    }

    // Window dimensions
    uint32_t x = window->x;
    uint32_t y = window->y;
    uint32_t width = window->width;
    uint32_t height = window->height;

    // Draw window background (white)
    draw_rect(fb, x, y, width, height, COLOR_WHITE);
    
    // Draw title bar (blue or gray)
    draw_rect(fb, x, y, width, WINDOW_TITLE_HEIGHT, window->is_active ? COLOR_BLUE : COLOR_GRAY);
    
    // Draw the window title
    draw_string(fb, window->title, x + 5, y + 8, COLOR_WHITE);
    
    // Draw window border (ASCII style)
    for (uint32_t i = 0; i < width; i++) {
        draw_pixel(fb, x + i, y, COLOR_GRAY); // Top border
        draw_pixel(fb, x + i, y + height - 1, COLOR_GRAY); // Bottom border
    }
    for (uint32_t i = 0; i < height; i++) {
        draw_pixel(fb, x, y + i, COLOR_GRAY); // Left border
        draw_pixel(fb, x + width - 1, y + i, COLOR_GRAY); // Right border
    }

    // Draw window controls (close button)
    // Close button represented by a simple "X"
    draw_rect(fb, x + width - 30, y + 5, 25, 25, COLOR_LIGHTGRAY);
    draw_string(fb, "X", x + width - 25, y + 10, COLOR_BLACK); // Close button
}

void set_active_window(WindowManager *wm, int window_id) {
    if (window_id >= 0 && window_id < wm->window_count) {
        // Deactivate current active window
        if (wm->active_window >= 0) {
            wm->windows[wm->active_window].is_active = false;
        }
        // Activate new window
        wm->active_window = window_id;
        wm->windows[window_id].is_active = true;
    }
}

void move_window(WindowManager *wm, int window_id, uint32_t new_x, uint32_t new_y) {
    if (window_id >= 0 && window_id < wm->window_count) {
        wm->windows[window_id].x = new_x;
        wm->windows[window_id].y = new_y;
    }
}