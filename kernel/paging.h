#ifndef DUNE_PAGING_H
#define DUNE_PAGING_H

#include "dune.h"

void paging_install(uintptr_t end);

uintptr_t phys_to_virt(uintptr_t phys);
uintptr_t virt_to_phys(uintptr_t virt);

#endif /* DUNE_PAGING_H */
