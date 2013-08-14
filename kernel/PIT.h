#ifndef PIT_H
#define PIT_H

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
 * 7-6: Channel # (0-2)
 * 5-4: RW (1=LSB, 2=MSB, 3=LSB then MSB)
 * 3-1: Mode
 * 0: BCD (0 = 16-bit counter, 1 = 4 digit BCD counter)
 *
 * Modes:
 * 0: Interrupt on terminal count
 * 1: Hardware Retriggerable one shot
 * 2: Rate Generator
 * 3: Square Wave Mode
 * 4: Software Strobe
 * 5: Hardware Strobe
 */


enum {
    PIT_DATA_REG0 = 0x40,
    PIT_DATA_REG2 = 0x42,
    PIT_CMD_REG   = 0x43,
    PIT_SPKR_REG  = 0x61,
    PIT_FREQ_HZ   = 1193189
};


#endif
