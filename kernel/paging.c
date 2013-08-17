#include "paging.h"

void paging_install(uintptr_t end)
{
    /* find first 4KB aligned address after the end of the kernel */
    /* create page directory for 4GB of RAM at this address */
    /* uint32_t *page_directory = (uint32_t*)((uint32_t)end & 0xFFFFF000) + 0x1000; */
    uint32_t page_directory = (end & 0xFFFFF000) + 0x1000;

    int i = 0;
    for (i = 0; i < 1024; i++) {
        ((uint32_t*)page_directory)[i] = 0 | 2; /* supervisor level, read/write, not present */
    }

    /* map first 4MB in first page table */
    uint32_t first_page_table = page_directory + 0x1000;
    uint32_t address = 0x0;
    for (i = 0; i < 1024; i++) {
        ((uint32_t*)first_page_table)[i] = address | 3;  /* supervisor level, read/write, present */
        address += 0x1000;  /* next page address */
    }

    ((uint32_t*)page_directory)[0] = first_page_table | 3; /* supervisor level, read/write, present */

    /* move page directory into cr3 */
    asm volatile("mov %0, %%cr3":: "b" (page_directory));

    /* read cr0, set paging bit, write it back */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=b" (cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "b" (cr0));

    uint32_t cr3;
    asm volatile("mov %%cr3, %0": "=b" (cr3));

    /*
    kprintf("Kernel end: 0x%x\n", end);
    kprintf("page directory phyical: 0x%x\n", cr3);
    kprintf("page directory entry 0: 0x%x\n", ((uint32_t*)cr3)[0]);
    uint32_t fpt = ((uint32_t*)cr3)[0] & 0xFFFFF000;
    kprintf("first page table physical: 0x%x\n", fpt);
    for (i = 0; i < 4; ++i) {
        kprintf("first page table entry %d: 0x%x\n", i, ((uint32_t*)fpt)[i]);
    }
    for (i = 1023; i > 1023-4; i--) {
        kprintf("first page table entry %d: 0x%x\n", i, ((uint32_t*)fpt)[i]);
    }
    */
}
