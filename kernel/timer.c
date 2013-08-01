#include <stdint.h>
#include <system.h>
#include <io.h>
#include <screen.h>
/*
 * Programmable Interval Timer (PIT 8253/8254) aka System Clock
 * Channel 0: IRQ 0
 * Channel 1: System specific
 * Channel 2: System speaker
 *
 * Default 18.222Hz
 *
 * Registers:
 * 0x40 - channel 0 data
 * 0x41 - channel 1 data
 * 0x42 - channel 2 data
 * 0x43 - command
 *
 * Command bits:
 * 7-6: Counter # (0-2)
 * 5-4: RW (1=LSB, 2=MSB, 3=LSB then MSB)
 * 3-1: Mode
 * 0: BCD (0 = 16-bit counter, 1 = 4xBCD decade counter)
 *
 * Modes:
 * 0: Interrupt on terminal count
 * 1: Hardware Retriggerable one shot
 * 2: Rate Generator
 * 3: Square Wave Mode
 * 4: Software Strobe
 * 5: Hardware Strobe
 */

void timer_phase(int hz)
{
    /* cmd = counter 0, LSB then MSB, Square Wave Mode, 16-bit counter */
    uint8_t cmd = 0x36; 
    int divisor = 1193180 / hz;
    outportb(0x43, cmd);            /* Set command byte */
    outportb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outportb(0x40, divisor >> 8);   /* Set high byte of divisor */
}

/* global count of system ticks (uptime) */
uint32_t timer_ticks = 0;

/* Handles timer interrupt.
 * By default, the timer fires at 18.222hz
 */
void timer_handler(struct regs *r)
{
    timer_ticks++;

    if (timer_ticks % 18 == 0) {
        //k_puts("One second has passed\n");
    }
}

/* installs timer_handler into IRQ0 */
void timer_install()
{
    irq_install_handler(0, timer_handler);
}

void delay(int ticks)
{
    uint32_t eticks = timer_ticks + ticks;
    while (timer_ticks < eticks)
        ;
}
