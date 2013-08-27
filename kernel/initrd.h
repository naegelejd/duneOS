#ifndef DUNE_INITRD_H
#define DUNE_INITRD_H

struct ramdisk {
    void* start_addr;
    size_t length;
};

void ramdisk_init(void* start, size_t len);

#endif /* DUNE_INITRD_H */
