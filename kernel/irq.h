#ifndef DUNE_IRQ_H
#define DUNE_IRQ_H

#include "int.h"

enum {
    IRQ_TIMER = 0,
    IRQ_KEYBOARD = 1,
    IRQ_RTC = 8
};

typedef void (*irq_handler)(struct regs *r);


void irq_install();
void irq_install_handler(int irq, irq_handler handler);

#endif /* DUNE_IRQ_H */
