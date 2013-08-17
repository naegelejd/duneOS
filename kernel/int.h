#ifndef DUNE_INT_H
#define DUNE_INT_H

#include "dune.h"

enum { EFLAGS_INTERRUPT_FLAG = 1 << 9 };

struct regs {
    /* pushed segs last */
    uint32_t gs, fs, es, ds;

    /* pushed by 'pusha' */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    /* pushed in ISR */
    uint32_t int_no, err_code;

    /* pushed by proc automatically */
    uint32_t epi, cs, eflags, useresp, ss;
};

bool interrupts_enabled(void);

bool beg_int_atomic(void);
void end_int_atomic(bool);

void kcli(void);
void ksti(void);

#endif /* DUNE_INT_H */
