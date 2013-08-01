#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memset(void *b, int c, size_t len);
void memcpy(char *dst, char *src, size_t n);
int strlen(char* s);

#endif
