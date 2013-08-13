#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

int kget_cursor();
void kset_cursor (unsigned int offset);
char kputc(char ch);
void kcls();
void kset_attr(uint8_t attr);

#endif
