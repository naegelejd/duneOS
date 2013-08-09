#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

int k_get_cursor ();
void k_set_cursor (unsigned int offset);
void kputc(char ch);
char k_putchar(unsigned char ch, uint8_t attr);
void k_clear_screen();
void k_puts(char *message);
void k_putnum(intptr_t);

#endif
