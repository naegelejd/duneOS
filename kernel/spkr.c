#include <io.h>
#include <system.h>
#include "PIT.h"

/* See PIT.h for comments on the Programmable Interval Timer */

void set_speaker_frequency(unsigned int hz)
{
    /* cmd = counter 2, LSB then MSB, Square Wave Mode, 16-bit counter */
    uint8_t cmd = 0xB6;
    unsigned int divisor = PIT_FREQ_HZ / hz;
    outportb(PIT_CMD_REG, cmd);            /* Set command byte */
    outportb(PIT_DATA_REG2, divisor); /* Set low byte of divisor */
    outportb(PIT_DATA_REG2, divisor >> 8);   /* Set high byte of divisor */
}

void beep(unsigned int ticks)
{
    uint8_t playing = inportb(PIT_SPKR_REG);
    uint8_t stop = inportb(PIT_SPKR_REG) & 0xFC;
    uint8_t start = playing | 3;

    set_speaker_frequency(44100);   /* 44.1 KHz */

    if (start != playing) {
        outportb(PIT_SPKR_REG, playing);
    }
    delay(ticks);

    outportb(PIT_SPKR_REG, stop);
}
