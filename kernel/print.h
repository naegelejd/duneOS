#ifndef PRINTK_H
#define PRINTK_H

#include <stddef.h>
#include <stdarg.h>

/* print a formatted string to terminal */
size_t kprintf(char *fmt, ...);

/* construct a formatted string in buffer 's'
 * no more than size bytes will be written.
 * if the result is < size bytes, the rest will be zero-padded
 */
size_t ksnprintf(char *s, size_t size, char *fmt, ...);

#endif /* PRINTK_H */
