#ifndef DUNE_ATA_H
#define DUNE_ATA_H

#include <stdint.h>

void ide_initialize(uint32_t bar0, uint32_t bar1, uint32_t bar2,
        uint32_t bar3, uint32_t bar4);

#endif /* DUNE_ATA_H */
