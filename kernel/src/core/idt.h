#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry {
    uint16_t base_low;    // Lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t selector;     // Kernel segment selector.
    uint8_t  always0;      // This must always be zero.
    uint8_t  flags;        // More flags.
    uint16_t base_high;   // Upper 16 bits of the address to jump to.
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;       // Size of the IDT - 1
    uint64_t base;        // Address of the first entry in the IDT
} __attribute__((packed));

void idt_install();
void idt_set_gate(uint8_t num, uint64_t base, uint16_t selector, uint8_t flags);

#endif // IDT_H