#ifndef DUNE_INITRD_H
#define DUNE_INITRD_H

#include "dune.h"
#include "blkdev.h"

struct ramdisk {
    uintptr_t start_addr;
    uintptr_t end_addr;
    size_t length;  /* convenience */
};

block_device_t* ramdisk_init(uintptr_t start, size_t len);

#endif /* DUNE_INITRD_H */
