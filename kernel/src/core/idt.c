#include "idt.h"

struct idt_entry idt[256]; // IDT has 256 entries
struct idt_ptr idtp;        // IDT pointer

void idt_set_gate(uint8_t num, uint64_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint64_t)&idt;
    
    asm volatile ("lidt %0" : : "m" (idtp)); // Load the IDT
}