#ifndef DUNE_INT_H
#define DUNE_INT_H

#include <stdbool.h>

enum { EFLAGS_INTERRUPT_FLAG = 1 << 9 };

struct regs;
typedef void (*int_handler_t)(struct regs *r);

bool interrupts_enabled(void);

bool beg_int_atomic(void);
void end_int_atomic(bool);

void cli(void);
void sti(void);

#endif /* DUNE_INT_H */
