#ifndef DUNE_IDT_H
#define DUNE_IDT_H

#include "int.h"

void idt_install();
void idt_set_int_gate(uint8_t num, uintptr_t base, unsigned dpl);
void int_install_handler(int interrupt, int_handler_t handler);

#endif /* DUNE_IDT_H */
