#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "display.h"
#include "core/gdt.h"
#include "core/idt.h"
#include "core/pic.h"
#include "desktop.h"
#include "lib/io.h"

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

    // Clear the framebuffer
    memset((void *)framebuffer->address, 0, framebuffer->pitch * framebuffer->height);;

    gdt_install();

    draw_string(framebuffer, "AxeniaOS V-1.0.0 x86_64", 1, 1, COLOR_WHITE);

    draw_string(framebuffer, "Loaded GDT.", 1, 40, COLOR_WHITE);

    idt_install();

    draw_string(framebuffer, "Loaded IDT.", 1, 60, COLOR_WHITE);

    pic_init();

    draw_string(framebuffer, "Loaded PIC.", 1, 80, COLOR_WHITE);

    draw_string(framebuffer, "Kernel Loaded.", 1, 100, COLOR_WHITE);

    display_axenia_menu(framebuffer);

    // Hang the system
    hcf();
}