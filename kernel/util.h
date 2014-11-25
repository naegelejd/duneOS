#ifndef DUNE_UTIL_H
#define DUNE_UTIL_H

#include "dune.h"

uint32_t get_esp(void);
uint32_t get_eflags(void);
uint32_t get_ring(void);
void kreboot(void);
void khalt(void);

#endif /* DUNE_UTIL_H */
