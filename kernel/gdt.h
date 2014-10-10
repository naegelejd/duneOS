#ifndef DUNE_GDT_H
#define DUNE_GDT_H

#include <stdint.h>

void set_kernel_stack(uint32_t sp);
void gdt_install();

#endif /* DUNE_GDT_H */
