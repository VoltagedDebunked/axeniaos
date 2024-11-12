#include "display.h"
#include "font.h"

void draw_pixel(struct limine_framebuffer *fb, uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb->width && y < fb->height) {
        volatile uint32_t *fb_ptr = fb->address;
        fb_ptr[y * (fb->pitch / 4) + x] = color;
    }
}

void draw_rect(struct limine_framebuffer *fb, uint32_t x, uint32_t y, 
               uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            draw_pixel(fb, x + j, y + i, color);
        }
    }
}

void draw_char(struct limine_framebuffer *fb, char c, uint32_t x, uint32_t y, uint32_t color) {
    if (c < 128) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (font[c][i] & (1 << j)) {
                    // Draw a larger pixel (2x2 block)
                    draw_pixel(fb, x + j * 2, y + i * 2, color);
                    draw_pixel(fb, x + j * 2 + 1, y + i * 2, color);
                    draw_pixel(fb, x + j * 2, y + i * 2 + 1, color);
                    draw_pixel(fb, x + j * 2 + 1, y + i * 2 + 1, color);
                }
            }
        }
    }
}

void draw_string(struct limine_framebuffer *fb, const char *str, uint32_t x, uint32_t y, uint32_t color) {
    while (*str) {
        draw_char(fb, *str, x, y, color);
        x += 12; // Change from 8 to 16 for doubled size
        str++;
    }
}