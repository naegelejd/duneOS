#ifndef SCREEN_H
#define SCREEN_H

int k_get_cursor ();
void k_set_cursor (unsigned int offset);
char k_putchar(char ch, unsigned char attr);
void k_clear_screen();
void k_puts(char *message);

#endif


