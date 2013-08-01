#include "system.h"
#include "screen.h"

void main() {
    k_clear_screen();
    k_puts("Welcome to DuneOS...\n\n");
    
    gdt_install();
    k_puts("GDT installed\n");

    idt_install();
    k_puts("IDT installed\n");
    k_puts("ISRs installed\n");

    irq_install();
    k_puts("IRQ handlers installed\n");

    __asm__ __volatile ("sti");
    k_set_cursor(0);
}
