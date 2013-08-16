#include <system.h>
#include <io.h>
#include <screen.h>
#include "PIT.h"

/* See PIT.h for comments on the Programmable Interval Timer */

void set_timer_frequency(unsigned int hz)
{
    /* cmd = channel 0, LSB then MSB, Square Wave Mode, 16-bit counter */
    uint8_t cmd = 0x36;
    unsigned int divisor = PIT_FREQ_HZ / hz;
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
    (void)r;    /* prevent 'unused' parameter warning */
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
    /* set frequency to 1 KHz... 100 Hz is more accurate */
    set_timer_frequency(1000);
    irq_install_handler(IRQ_TIMER, timer_handler);
}

void delay(unsigned int ticks)
{
    unsigned int eticks = timer_ticks + ticks;
    while (timer_ticks < eticks)
        ;
}
