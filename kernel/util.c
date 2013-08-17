#include "util.h"

/* hack a hard reset by loading a bogus IDT */
uint32_t no_idt[2] = {0, 0};
void kreboot()
{
    asm volatile ("lidt no_idt");
}

void khalt()
{
    asm volatile ("cli\nhlt");
}

void print_esp()
{
    uint32_t esp;
    asm volatile ("mov %%esp, %0\n" : "=o" (esp) :);
    kprintf("esp: 0x%x\n", esp);
}
