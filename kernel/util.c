#include "system.h"


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
