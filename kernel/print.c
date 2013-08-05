#include <stdbool.h>
#include "print.h"
#include "screen.h" /* for kputc */

typedef void (*cput_t) (char **, char);

/* calls kputc to put a char on the screen */
static void putc_screen(char **s, char c)
{
    kputc(c);
}

/* appends a char to a string */
static void putc_string(char **s, char c)
{
    *(*s)++ = c;
}

enum { NUM_BUF_SIZE = 12 };

static void uint2str(char *buf, unsigned int num, unsigned int base, bool uppercase)
{
    unsigned int n = 0;
    unsigned int d = 1;
    while ((num / d) >= base) {
        d *= base;
    }
    while (d != 0) {
        int digit = num / d;
        num %= d;
        d /= base;
        if (n > 0 || digit > 0 || d == 0) {
            *buf++ = digit + (digit < 10 ? '0' : (uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buf = 0;
}

static int a2d(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return -1;
    }
}

static unsigned int str2uint(char **s, int base)
{
    int digit = 0, num = 0;
    while ((digit = a2d(*(*s)++)) >= 0) {
        if (digit > base) {
            break;
        }
        num = num * base + digit;
    }
    return num;
}

static void put_string(char **s, cput_t put, char *out, unsigned int pad, bool ldng_zero)
{
    char c = 0;
    char padding = ldng_zero ? '0' : ' ';   /* determine the type of left-padding */

    /* walk through the string, determining if it is longer/shorter than padding amount */
    char *pass = out;
    while (*pass++ && (pad > 0)) {
        pad--;
    }

    /* actually put the padding char */
    while (pad-- > 0) {
        put(s, padding);
    }

    /* print the string */
    while ((c = *out++)) {
        put(s, c);
    }
}


static size_t vkprint(char **s, size_t size, cput_t put, char *fmt, va_list arg)
{
    char buffer[NUM_BUF_SIZE];
    char c;
    size_t len = 0;
    size = size - 1;    /* if size is 0, size - 1 is huge (unsigned) */

    while ((c = *fmt++)) {
        if (c != '%') {
            put(s, c);
            len++;
        } else {
            bool ldng_zero = false;
            unsigned int pad = 0;
            c = *fmt++;
            if (c == '0') {
                ldng_zero = true;
                c = *fmt++;
            } else if (c > '0' && c < '9') {
                --fmt;
                pad = str2uint(&fmt, 10);
            }
            switch (c) {

                case 0:
                    goto vkprint_end;
                    break;

                case '%':
                    put(s, c);
                    break;

                case 'x': 
                case 'X':
                    uint2str(buffer, va_arg(arg, unsigned int), 16, (c == 'X'));
                    put_string(s, put, buffer, pad, ldng_zero);
                    break;

                case 'd': {
                    int x = va_arg(arg, int);
                    if (x < 0) {
                        put(s, '-');
                        x = -x;
                    }
                    uint2str(buffer, x, 10, false);
                    put_string(s, put, buffer, pad, ldng_zero);
                    break;
                }

                case 'u':
                    uint2str(buffer, va_arg(arg, unsigned int), 10, false);
                    put_string(s, put, buffer, pad, ldng_zero);
                    break;

                case 's':
                    put_string(s, put, va_arg(arg, char*), 0, false);
                    break;
                
                case 'c':
                    put(s, (char)va_arg(arg, int));
                    break;

                default:
                    ;
            }
            len++;
        }

        /*
        if (len >= size) {
            put(s, 0);
            goto vkprint_end
        }
        */
    }
vkprint_end:
    return len;
}


size_t kprintf(char *fmt, ...)
{
    va_list arg;
    size_t sz;

    va_start(arg, fmt);
    sz = vkprint(NULL, 0, putc_screen, fmt, arg);
    va_end(arg);

    return sz;
}

size_t ksnprintf(char *s, size_t size, char *fmt, ...)
{
    va_list arg;
    size_t sz;

    va_start(arg, fmt);
    sz = vkprint(&s, size, putc_string, fmt, arg);
    va_end(arg);

    return sz;
}
