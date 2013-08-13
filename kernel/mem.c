#include "system.h"
#include "print.h"

void mem_init(uintptr_t start, uintptr_t end)
{
    uint32_t npages = (end - start) / 0x1000;
    kprintf("Number of pages: %u\n", npages);
}
