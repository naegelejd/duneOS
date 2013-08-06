#include <stdint.h>
#include "string.h"

void *memset(void *b, int c, size_t len)
{
    uint8_t *s = b;
    while (--len) {
        *s++ = (uint8_t)c;
    }

    return b;
}

void memcpy(char *dst, char *src, size_t n)
{
    size_t i = 0;
    for (i = 0; i < n; i++) {
        *(dst + i) = *(src + i);
    }
}

int strlen(char *s)
{
    int l = 0;
    while (*s++) {
        l++;
    }
    return l;
}
