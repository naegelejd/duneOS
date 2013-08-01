#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

/*
 * Read byte from port
 */
unsigned char inportb (unsigned short port);

/*
 * Write byte to port
 */
void outportb (unsigned short port, unsigned char data);

/*
 * Read word from port
 */
unsigned short inportw (unsigned short port);

/*
 * Write word to port
 */
void outportw (unsigned short port, unsigned short data);

/*
 * Short delay.  May be needed when talking to some
 * (slow) I/O devices.
 */
void io_delay(void);

#endif
