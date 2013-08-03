#include <system.h>
#include <io.h>
#include <screen.h>
#include "PIT.h"

/* See PIT.h for comments on the Programmable Interval Timer */

void timer_phase(int hz)
{
    /* cmd = counter 0, LSB then MSB, Square Wave Mode, 16-bit counter */
    uint8_t cmd = 0x36; 
    int divisor = 1193180 / hz;
    outportb(PIT_CMD_REG, cmd);            /* Set command byte */
    outportb(PIT_DATA_REG0, divisor & 0xFF); /* Set low byte of divisor */
    outportb(PIT_DATA_REG0, divisor >> 8);   /* Set high byte of divisor */
}

/* global count of system ticks (uptime) */
uint32_t timer_ticks = 0;

/* Handles timer interrupt.
 * By default, the timer fires at 18.222hz
 */
void timer_handler(struct regs *r)
{
    timer_ticks++;
    /*
    if (timer_ticks % 1000 == 0) {
        k_puts("One second has passed\n");
    }
    */
}

/* installs timer_handler into IRQ0 */
void timer_install()
{
    timer_phase(1000);
    irq_install_handler(0, timer_handler);
}

void delay(unsigned int ticks)
{
    unsigned int eticks = timer_ticks + ticks;
    while (timer_ticks < eticks)
        ;
}
