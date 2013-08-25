#ifndef DUNE_MEM_H
#define DUNE_MEM_H

#include "dune.h"
#include "multiboot.h"

enum {
    PAGE_POWER = 12,
    PAGE_SIZE = (unsigned)(1 << PAGE_POWER),
    PAGE_MASK = (~(PAGE_SIZE - 1))
};

enum {
    PAGE_AVAIL  = 0x1,  /* page on freelist */
    PAGE_KERN   = 0x2,  /* page used by kernel */
    PAGE_HDWARE = 0x4,  /* page used by hardware (ISA hole) */
    PAGE_ALLOC  = 0x8,  /* page allocated */
    PAGE_UNUSED = 0x10, /* page unused */
    PAGE_HEAP   = 0x20  /* page in kernel heap */
};

enum {
    HDWARE_RAM_START = 0x9F000, /* 0x9FC00 page aligned */
    HDWARE_RAM_END   = 0x100000 /* end of Video memory */
};

enum {
    KERNEL_HEAP_SIZE = 0x100000    /* 1M heap */
};

struct page {
    uint32_t flags;
    struct page *next;
};
typedef struct page page_t;


void mem_init(struct multiboot_info *mbinfo, uintptr_t kernstart, uintptr_t kernend);
void bss_init(void);

void* alloc_page(void);
void free_page(void* page_addr);

void* malloc(size_t size);
void free(void *buffer);

#endif /* DUNE_MEM_H */
