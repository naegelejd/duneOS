#ifndef DUNE_IRQ_H
#define DUNE_IRQ_H

#include "int.h"

enum {
    IRQ_TIMER = 0,
    IRQ_KEYBOARD = 1,
    IRQ_RTC = 8,
    IRQ_MOUSE = 12
};

void irq_install();
void irq_install_handler(unsigned int irq, int_handler_t handler);
void enable_irq(unsigned int irq);
void disable_irq(unsigned int irq);
bool irq_enabled(unsigned int irq);

#endif /* DUNE_IRQ_H */
