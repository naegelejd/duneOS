#ifndef DUNE_IRQ_H
#define DUNE_IRQ_H

#include "int.h"


enum { NUM_IRQ_HANDLERS = 16 };
enum { IRQ_ISR_START = 32 };

enum {
    IRQ_TIMER = 0,
    IRQ_KEYBOARD = 1,
    IRQ_RTC = 8
};

void irq_install();
void irq_install_handler(unsigned int irq, int_handler_t handler);
void enable_irq(unsigned int irq);
void disable_irq(unsigned int irq);

#endif /* DUNE_IRQ_H */
