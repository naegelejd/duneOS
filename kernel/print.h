#ifndef DUNE_PRINT_H
#define DUNE_PRINT_H

#include "dune.h"

/* print a formatted string to terminal */
size_t kprintf(char *fmt, ...);

/* construct a formatted string in buffer 's' */
size_t ksprintf(char *s, char *fmt, ...);

/* **TEMPORARY** UNBUFFERED userspace printf */
size_t uprintf(char *fmt, ...);

#ifdef QEMU_DEBUG

#define DEBUG(s) \
    do { \
        dbgprintf("%s:%u: " s, __FILE__, __LINE__); \
    } while (0)

#define DEBUGF(fmt, ...) \
    do { \
        dbgprintf("%s:%u: " fmt, __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

/* print a formatted string to QEMU's debug console */
size_t dbgprintf(char *fmt, ...);

#else /* QEMU_DEBUG */
#   define DEBUG(...)
#   define dbgprintf(...)
#endif /* QEMU_DEBUG */

#endif /* DUNE_PRINT_H */
