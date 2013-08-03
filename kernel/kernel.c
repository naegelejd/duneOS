#include "system.h"
#include "screen.h"
#include "kb.h"

void main() {
    k_clear_screen();
    k_puts("Welcome to DuneOS...\n\n");
    
    gdt_install();
    k_puts("GDT installed\n");

    idt_install();
    k_puts("IDT installed\n");
    k_puts("ISRs installed\n");

    irq_install();
    timer_install();
    keyboard_install();
    k_puts("IRQ handlers installed\n");

    __asm__ __volatile ("sti");


    /* debugging */
    extern intptr_t start;
    extern intptr_t code;
    extern intptr_t data;
    extern intptr_t bss;
    extern intptr_t end;
    k_puts("Kernel size in bytes: ");
    k_putnum(&end - &start);
    k_putchar('\n', 0);

    k_puts("Start: ");
    k_putnum(&start);
    k_putchar('\n', 0);

    k_puts("Code: ");
    k_putnum(&code);
    k_putchar('\n', 0);

    k_puts("Data: ");
    k_putnum(&data);
    k_putchar('\n', 0);

    k_puts("BSS: ");
    k_putnum(&bss);
    k_putchar('\n', 0);

    k_puts("End: ");
    k_putnum(&end);
    k_putchar('\n', 0);

    /* playing around */
    k_set_cursor(0);
    beep(2);
    delay(3000);
    reboot();
}

/* hack a hard reset by loading a bogus IDT */
uint32_t no_idt[2] = {0, 0};
void reboot()
{
    __asm__ ("lidt no_idt");
}

void halt()
{
    __asm__ ("cli\nhlt");
}
