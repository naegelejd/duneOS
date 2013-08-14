#ifndef DUNE_MEM_H
#define DUNE_MEM_H

#include "dune.h"

void mem_init(struct multiboot_info *mbinfo);
void bss_init(void);

void* alloc_page(void);
void free_page(void* page_addr);

void* malloc(size_t size);
void free(void *buffer);

#endif /* DUNE_MEM_H */
