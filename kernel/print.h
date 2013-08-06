#ifndef PRINTK_H
#define PRINTK_H

#include <stddef.h>
#include <stdarg.h>

/* print a formatted string to terminal */
size_t kprintf(char *fmt, ...);

/* construct a formatted string in buffer 's' */
size_t ksprintf(char *s, char *fmt, ...);

#endif /* PRINTK_H */
