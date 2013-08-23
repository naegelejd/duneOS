#ifndef DUNE_INT_H
#define DUNE_INT_H

#include "dune.h"

enum { EFLAGS_INTERRUPT_FLAG = 1 << 9 };

struct regs {
    /* pushed segs last */
    uint32_t gs, fs, es, ds;

    /* pushed by 'pusha' */
    /* uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; */
    uint32_t edi, esi, ebp, ebx, edx, ecx, eax;

    /* pushed in ISR */
    uint32_t int_no, err_code;

    /* pushed by proc automatically */
    uint32_t eip, cs, eflags;

    /* pushed in user mode? */
    /* uint32_t useresp, ss; */
};

typedef void (*int_handler_t)(struct regs *r);

bool interrupts_enabled(void);

bool beg_int_atomic(void);
void end_int_atomic(bool);

void cli(void);
void sti(void);

#endif /* DUNE_INT_H */
