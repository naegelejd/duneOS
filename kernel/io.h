#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

#include <stdint.h>

/*
 * Read byte from port
 */
uint8_t inportb (uint16_t port);

/*
 * Write byte to port
 */
void outportb (uint16_t port, uint8_t data);

/*
 * Read word from port
 */
uint16_t inportw (uint16_t port);

/*
 * Write word to port
 */
void outportw (uint16_t port, uint16_t data);

/*
 * Short delay.  May be needed when talking to some
 * (slow) I/O devices.
 */
void io_delay(void);

#endif
