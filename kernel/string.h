#ifndef DUNE_STRING_H
#define DUNE_STRING_H

#include "dune.h"

void *memset(void *b, int c, size_t len);
void memcpy(void *dst, const void *src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
int strlen(char* s);

#endif /* DUNE_STRING_H */
