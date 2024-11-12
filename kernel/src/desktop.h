#ifndef DESKTOP_H
#define DESKTOP_H

#include <limine.h>
#include "window.h"

#define TASKBAR_HEIGHT 20
#define BUTTON_WIDTH 60
#define BUTTON_HEIGHT 18

typedef struct {
    WindowManager *wm;
    char time[6];  // "HH:MM\0"
} Desktop;

void init_desktop(Desktop *desktop, WindowManager *wm);
void draw_desktop_background(struct limine_framebuffer *fb);
void draw_taskbar(struct limine_framebuffer *fb, Desktop *desktop);
void draw_desktop(struct limine_framebuffer *fb, Desktop *desktop);

#endif // DESKTOP_H