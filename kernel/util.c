#include "system.h"


/* hack a hard reset by loading a bogus IDT */
uint32_t no_idt[2] = {0, 0};
void reboot()
{
    asm volatile ("lidt no_idt");
}

void halt()
{
    asm volatile ("cli\nhlt");
}

void cli()
{
    asm volatile ("cli");
}

void sti()
{
    asm volatile("sti");
}
