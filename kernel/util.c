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

uint32_t get_esp()
{
    uint32_t esp;
    asm volatile ("mov %%esp, %0\n" : "=o" (esp) :);
    return esp;
}
