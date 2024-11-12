#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <limine.h>

// Color definitions
#define COLOR_BLACK    0x000000
#define COLOR_WHITE    0xFFFFFF
#define COLOR_GRAY     0x808080
#define COLOR_BLUE     0x0000FF
#define COLOR_LIGHTGRAY 0xD3D3D3
#define COLOR_DARKGRAY  0x404040

// Basic drawing functions
void draw_pixel(struct limine_framebuffer *fb, uint32_t x, uint32_t y, uint32_t color);
void draw_rect(struct limine_framebuffer *fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_char(struct limine_framebuffer *fb, char c, uint32_t x, uint32_t y, uint32_t color);
void draw_string(struct limine_framebuffer *fb, const char *str, uint32_t x, uint32_t y, uint32_t color);

#endif // DISPLAY_H