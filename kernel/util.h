#ifndef DUNE_UTIL_H
#define DUNE_UTIL_H

#include "dune.h"

uint32_t get_esp();
uint32_t get_eflags();
void kreboot();
void khalt();

#endif /* DUNE_UTIL_H */
