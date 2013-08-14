#include "int.h"

extern uint32_t get_eflags(void);

bool interrupts_enabled(void)
{
    uint32_t eflags = get_eflags();
    return (eflags & EFLAGS_INTERRUPT_FLAG) != 0;
}

void kcli()
{
    asm volatile ("cli");
}

void ksti()
{
    asm volatile("sti");
}

bool beg_int_atomic(void)
{
    bool enabled = interrupts_enabled();
    if (enabled) {
        kcli();
    }
    return enabled;
}

void end_int_atomic(bool iflag)
{
    KASSERT(!interrupts_enabled());
    if (iflag) {
        ksti();
    }
}
