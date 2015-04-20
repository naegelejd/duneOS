#ifndef DUNE_IO_H
#define DUNE_IO_H

#include <stdint.h>

/* Read byte from port */
uint8_t inportb(uint16_t port);

/* Write byte to port */
void outportb(uint16_t port, uint8_t data);

/* Read word from port */
uint16_t inportw(uint16_t port);

/* Write word to port */
void outportw(uint16_t port, uint16_t data);

/* Read double word from port */
uint32_t inportl(uint16_t port);

/* Write double word to port */
uint32_t outportl(uint16_t port, uint32_t data);

/* Read `count` words from `port` into `buffer` */
void inportsw(uint16_t port, uintptr_t buffer, unsigned int count);

/* Write `count` words from `buffer` to `port` */
void outportsw(uint16_t port, uintptr_t buffer, unsigned int count);

/* Short delay. May be needed when talking to some (slow) I/O devices. */
void io_delay(void);

#endif /* DUNE_IO_H */
