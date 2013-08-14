#ifndef DUNE_INT_H
#define DUNE_INT_H

#include "dune.h"

enum { EFLAGS_INTERRUPT_FLAG = 1 << 9 };

bool interrupts_enabled(void);

bool beg_int_atomic(void);
void end_int_atomic(bool);

void kcli(void);
void ksti(void);

#endif /* DUNE_INT_H */
