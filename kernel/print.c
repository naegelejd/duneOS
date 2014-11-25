#include <stdarg.h>
#include "print.h"
#include "string.h"
#include "screen.h" /* for kputc */


enum { NUM_BUF_SIZE = 12 };

static size_t uint2str(char *, unsigned int, unsigned int, bool);
static int a2d(char);
static unsigned int str2uint(char **, int);

typedef void (cput_t)(char **, char);
static void put_string(char **, cput_t, char *, int, bool);
static size_t kvasprintf(char **, cput_t, char *, va_list);

static size_t uint2str(char *buf, unsigned int num, unsigned int base, bool uppercase)
{
    size_t n = 0;
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
    return n;
}

static int a2d(char c)
{
    /* kprintf("c: %c\n", c); */
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
    char *p = *s;
    while ((digit = a2d(*p++)) >= 0) {
        /* kprintf("dgt: %d\n", digit); */
        if (digit > base) {
            break;
        }
        num = num * base + digit;
    }

    /* rewind one char since it wasn't numeric and fast-forward 's' */
    *s = --p;
    return num;
}

static void put_string(char **s, cput_t put, char *out, int pad, bool ldng_zero)
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


static size_t kvasprintf(char **dest, cput_t put, char *fmt, va_list arg)
{
    char num_buf[NUM_BUF_SIZE] = "\0";      /* for reading numbers */
    char* str_buf = NULL;                   /* for reading strings */
    char c = 0;
    size_t length = 0, count = 0;

    while ((c = *fmt++)) {
        bool ldng_zero = false;
        unsigned int pad = 0;

        if (c != '%') {
            num_buf[0] = c;
            num_buf[1] = 0;
            count = 2;
        } else {
            /* eat up leading zeros */
            while ((c = *fmt++) == '0') {
                ldng_zero = true;
            }

            /* determine the 'amount' of padding */
            if (c > '0' && c < '9') {
                --fmt;
                pad = str2uint(&fmt, 10);
                c = *fmt++;
            }

            /* handle format specifier */
            switch (c) {

                case 0:
                    goto vkprint_end;
                    break;

                case '%':
                    num_buf[0] = c;
                    num_buf[1] = 0;
                    count = 2;
                    break;

                case 'x':
                case 'X':
                    count = uint2str(num_buf, va_arg(arg, unsigned int),
                            16, (c == 'X'));
                    break;

                case 'd': {
                    int x = va_arg(arg, int);
                    if (x < 0) {
                        num_buf[0] = '-';
                        x = -x;
                        count = uint2str(num_buf + 1, x, 10, false);
                    } else {
                        count = uint2str(num_buf, x, 10, false);
                    }
                    break;
                }

                case 'u':
                    count = uint2str(num_buf, va_arg(arg, unsigned int), 10, false);
                    break;

                case 's': {
                    str_buf = va_arg(arg, char*);
                    count = strlen(str_buf);
                    break;
                }

                case 'c':
                    num_buf[0] = c;
                    num_buf[1] = 0;
                    count = 2;
                    break;

                default:
                    ;
            }
        }

        length += count;
        if (str_buf) {
            put_string(dest, put, str_buf, pad, ldng_zero);
            str_buf = NULL;
        } else {
            put_string(dest, put, num_buf, pad, ldng_zero);
        }
    }
vkprint_end:
    return length;
}

/* appends a char to a string */
static void putc_string(char **s, char c)
{
    *(*s)++ = c;
}

/* prints a formatted string to a char buffer */
size_t ksprintf(char *s, char *fmt, ...)
{
    va_list arg;
    size_t sz;

    va_start(arg, fmt);
    sz = kvasprintf(&s, putc_string, fmt, arg);
    va_end(arg);

    return sz;
}

/* calls kputc to put a char on the screen */
static void putc_screen(char **s, char c)
{
    (void)s;    /* prevent 'unused parameter' warning */
    kputc(c);
}

/* prints a formatted string to a the console */
size_t kprintf(char *fmt, ...)
{
    va_list arg;
    size_t sz;

    va_start(arg, fmt);
    sz = kvasprintf(NULL, putc_screen, fmt, arg);
    va_end(arg);

    return sz;
}


#include "syscall.h"
void uputc(char **s, char c)
{
    (void)s;
    char buf[2];
    buf[0] = c;
    syscall_print(buf);
}

#include <stdarg.h>
size_t uprintf(char *fmt, ...)
{
    va_list arg;
    size_t sz;
    va_start(arg, fmt);
    sz = kvasprintf(NULL, uputc, fmt, arg);
    va_end(arg);
    return sz;
}


#ifdef QEMU_DEBUG

#include "io.h"

enum { QEMU_DEBUG_PORT = 0xE9 };

static void putc_debug(char **s, char c)
{
    (void)s;    /* prevent 'unused parameter' warning */
    outportb(QEMU_DEBUG_PORT, c);
}

size_t dbgprintf(char *fmt, ...)
{
    va_list arg;
    size_t sz;

    va_start(arg, fmt);
    sz = kvasprintf(NULL, putc_debug, fmt, arg);
    va_end(arg);

    return sz;
}

#endif /* QEMU_DEBUG */
