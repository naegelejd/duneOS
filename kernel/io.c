#include "io.h"

uint8_t inportb (uint16_t port)
{
    uint8_t result;
    asm volatile ("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void outportb (uint16_t port, uint8_t data)
{
    asm volatile ("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t inportw (uint16_t port)
{
    uint16_t result;
    asm volatile ("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void outportw (uint16_t port, uint16_t data)
{
    asm volatile ("out %%ax, %%dx" : : "a" (data), "d" (port));
}

void io_delay(void)
{
    uint8_t value = 0;
    asm volatile ("outb %0, $0x80" : : "a" (value));
}
