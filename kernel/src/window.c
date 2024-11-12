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

    // Window background
    draw_rect(fb, window->x, window->y, window->width, window->height, COLOR_WHITE);
    
    // Title bar
    draw_rect(fb, window->x, window->y, window->width, WINDOW_TITLE_HEIGHT, 
              window->is_active ? COLOR_BLUE : COLOR_GRAY);
    draw_string(fb, window->title, window->x + 5, window->y + 8, COLOR_WHITE);
    
    // Window border
    for (uint32_t i = 0; i < window->height; i++) {
        draw_pixel(fb, window->x - 1, window->y + i, COLOR_GRAY);
        draw_pixel(fb, window->x + window->width, window->y + i, COLOR_GRAY);
    }
    for (uint32_t i = 0; i < window->width + 1; i++) {
        draw_pixel(fb, window->x + i - 1, window->y - 1, COLOR_GRAY);
        draw_pixel(fb, window->x + i - 1, window->y + window->height, COLOR_GRAY);
    }
    
    // Window controls (close button)
    draw_rect(fb, window->x + window->width - 20, window->y + 5, 15, 15, COLOR_LIGHTGRAY);
    draw_string(fb, "x", window->x + window->width - 15, window->y + 7, COLOR_BLACK);
}

void draw_all_windows(struct limine_framebuffer *fb, WindowManager *wm) {
    // Draw windows from back to front
    for (int i = 0; i < wm->window_count; i++) {
        draw_window(fb, &wm->windows[i]);
    }
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