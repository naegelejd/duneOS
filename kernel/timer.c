#include "irq.h"
#include "io.h"
#include "PIT.h"
#include "thread.h"
#include "timer.h"

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
static uint32_t g_num_ticks = 0;


/* getter for global system tick count */
uint32_t get_ticks(void)
{
    return g_num_ticks;
}

/* Handles timer interrupt.
 * By default, the timer fires at 18.222hz
 */
void timer_handler(struct regs *r)
{
    (void)r;    /* prevent 'unused' parameter warning */
    g_num_ticks++;

    /* if the current thread has outlived the quantum, add it to the
     * run queue and schedule a new thread */
    thread_t* current = get_current_thread();
    if (++current->num_ticks > THREAD_QUANTUM) {
        make_runnable(current);
        schedule();
    }
}

/* installs timer_handler into IRQ0 */
void timer_install()
{
    /* set frequence to 100 Hz */
    set_timer_frequency(100);
    irq_install_handler(IRQ_TIMER, timer_handler);
    enable_irq(IRQ_TIMER);
}

void delay(unsigned int ticks)
{
    unsigned int eticks = g_num_ticks + ticks;
    while (g_num_ticks < eticks)
        ;
}
