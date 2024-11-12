#include "gdt.h"

struct gdt_entry gdt[5];
struct gdt_ptr gdtp;

void gdt_set_gate(int num, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_install() {
    gdtp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdtp.base = (uint64_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    asm volatile (
        "lgdt %0\n"         // Load GDT
        "movw $0x10, %%ax\n" // Move segment selector to ax
        "movw %%ax, %%ds\n" // Set DS
        "movw %%ax, %%es\n" // Set ES
        "movw %%ax, %%fs\n" // Set FS
        "movw %%ax, %%gs\n" // Set GS
        "movw %%ax, %%ss\n" // Set SS
        :
        : "m" (gdtp)        // Use "m" to indicate memory reference
        : "memory"
    );
}