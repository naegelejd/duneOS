#ifndef DUNE_X86_H
#define DUNE_X86_H

#include <stdint.h>

enum {
    NULL_SEG_SELECTOR = 0x0,
    CODE_SEG_SELECTOR = 0x08,
    DATA_SEG_SELECTOR = 0x10,
    USER_CODE_SEG_SELECTOR = 0x18,
    USER_DATA_SEG_SELECTOR = 0x20,
    TSS_SELECTOR = 0x28
};

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


enum { KERNEL_DPL = 0, USERMODE_DPL = 3 };


#endif /* DUNE_X86_H */
