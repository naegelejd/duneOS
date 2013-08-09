#include "system.h"
#include "print.h"

void paging_install(uint32_t* end)
{
    /* find first 4KB aligned address after the end of the kernel */
    /* create page directory for 4GB of RAM at this address */
    uint32_t *page_directory = (uint32_t*)((uint32_t)end & 0xFFFFF000) + 0x1000;
    //kprintf("Page Directory location: 0x%x\n", page_aligned_end);

    int i = 0;
    for (i = 0; i < 1024; i++) {
        page_directory[i] = 0 | 2; /* supervisor level, read/write, not present */
    }

    /* map first 4MB in first page table */
    uint32_t *first_page_table = page_directory + 0x1000;
    uint32_t address = 0x0;
    for (i = 0; i < 1024; i++) {
        first_page_table[i] = address | 2;  /* supervisor level, read/write, present */
        address += 0x1000;  /* next page address */
    }

    page_directory[0] = (uint32_t)first_page_table | 3; /* supervisor level, read/write, present */

    /* move page directory into cr3 */
    asm volatile("mov %0, %%cr3":: "b" (page_directory));

    /* read cr0, set paging bit, write it back */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=b" (cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr3":: "b" (cr0));
}
