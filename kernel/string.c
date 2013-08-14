#include "string.h"

void *memset(void *b, int c, size_t len)
{
    uint8_t *s = b;
    while (--len) {
        *s++ = (uint8_t)c;
    }

    return b;
}

void memcpy(void *vdst, const void *vsrc, size_t n)
{
    const unsigned char* src = vsrc;
    unsigned char* dst = vdst;

    size_t i = 0;
    for (i = 0; i < n; i++) {
        *(dst + i) = *(src + i);
    }
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    const unsigned char* p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        } else {
            p1++;
            p2++;
        }
    }
    return 0;
}

int strlen(char *s)
{
    int l = 0;
    while (*s++) {
        l++;
    }
    return l;
}
