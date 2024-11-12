#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include <limine.h>
#include <stdbool.h>

#define WINDOW_TITLE_HEIGHT 25
#define MAX_WINDOWS 16

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    char title[64];
    bool is_active;
    bool is_visible;
} Window;

typedef struct {
    Window windows[MAX_WINDOWS];
    int window_count;
    int active_window;
} WindowManager;

// Window management functions
void init_window_manager(WindowManager *wm);
int create_window(WindowManager *wm, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const char *title);
void draw_window(struct limine_framebuffer *fb, Window *window);
void draw_all_windows(struct limine_framebuffer *fb, WindowManager *wm);
void set_active_window(WindowManager *wm, int window_id);
void move_window(WindowManager *wm, int window_id, uint32_t new_x, uint32_t new_y);

#endif // WINDOW_H