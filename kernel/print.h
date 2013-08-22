#ifndef DUNE_PRINT_H
#define DUNE_PRINT_H

#include "dune.h"

/* print a formatted string to terminal */
size_t kprintf(char *fmt, ...);

/* construct a formatted string in buffer 's' */
size_t ksprintf(char *s, char *fmt, ...);

#ifdef QEMU_DEBUG

/* print a formatted string to QEMU's debug console */
size_t dbgprintf(char *fmt, ...);

#else /* QEMU_DEBUG */
#   define dbgprintf(...)
#endif /* QEMU_DEBUG */

#endif /* DUNE_PRINT_H */
