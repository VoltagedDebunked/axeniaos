#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "display.h"
#include "window.h"
#include "desktop.h"

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*)ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

// Limine requests
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static void hcf(void) {
    for (;;) {
        #if defined (__x86_64__)
            asm ("hlt");
        #elif defined (__aarch64__) || defined (__riscv)
            asm ("wfi");
        #elif defined (__loongarch64)
            asm ("idle 0");
        #endif
    }
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Initialize window manager and desktop
    WindowManager wm;
    Desktop desktop;
    
    init_window_manager(&wm);
    init_desktop(&desktop, &wm);

    // Create some sample windows
    create_window(&wm, 100, 100, 400, 300, "Welcome to MyOS");
    create_window(&wm, 150, 150, 300, 200, "Terminal");
    create_window(&wm, 200, 200, 350, 250, "File Manager");

    // Clear the framebuffer
    memset((void *)framebuffer->address, 0, framebuffer->pitch * framebuffer->height);

    // Draw the complete desktop
    draw_desktop(framebuffer, &desktop);

    // Hang the system
    hcf();
}