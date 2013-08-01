#include "io.h"

unsigned char inportb (unsigned short port)
{
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void outportb (unsigned short port, unsigned char data)
{
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

unsigned short inportw (unsigned short port)
{
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void outportw (unsigned short port, unsigned short data)
{
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

void io_delay(void)
{
    unsigned char value = 0;
    __asm__ __volatile__ ("outb %0, $0x80" : : "a" (value));
}
