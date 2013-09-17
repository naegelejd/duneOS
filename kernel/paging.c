#include "mem.h"
#include "paging.h"

uintptr_t phys_to_virt(uintptr_t phys)
{
    return phys + KERNEL_VBASE;
}

uintptr_t virt_to_phys(uintptr_t virt)
{
    return virt - KERNEL_VBASE;
}

void paging_install(void)
{
    /* find first 4KB aligned address after the end of the kernel */
    /* create page directory for 4GB of RAM at this address */
    uintptr_t page_directory = (uintptr_t)alloc_page();
    KASSERT(page_directory);
    DEBUGF("page directory: 0x%x\n", virt_to_phys(page_directory));

    unsigned int pde = 0;
    /* set bits for all page directory entries */
    for (pde = 0; pde < 1024; pde++) {
        ((uintptr_t*)page_directory)[pde] = 0 | 2; /* supervisor level, read/write, not present */
    }

    /* map first 16MB in 4 page tables */
    uintptr_t address = 0x0;
    unsigned int pidx = 0;
    for (pidx = KERNEL_VBASE >> 22; pidx < (KERNEL_VBASE >> 22) + 4; pidx++) {
        uintptr_t page_table = (uintptr_t)alloc_page();
        KASSERT(page_table);

        unsigned int pte;
        for (pte = 0; pte < 1024; pte++) {
            ((uint32_t*)page_table)[pte] = address | 3;  /* supervisor level, read/write, present */
            address += 0x1000;  /* next page address */
        }

        /* set PHYSICAL addresses of page table(s) in page directory */
        /* supervisor level, read/write, present */
        ((uintptr_t*)page_directory)[pidx] = virt_to_phys(page_table) | 3;
        DEBUGF("page table %u: 0x%x\n", pidx, virt_to_phys(page_table));
    }

    /* move PHYSICAL page directory address into cr3 */
    asm volatile("mov %0, %%cr3":: "b" (virt_to_phys(page_directory)));

    /* clear 4MB page bit since we're switching to 4KB pages */
    uint32_t cr4;
    asm volatile("mov %%cr4, %0": "=b" (cr4));
    cr4 &= ~(0x00000010);
    asm volatile("mov %0, %%cr4":: "b" (cr4));

    /* read cr0, set paging bit, write it back */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=b" (cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "b" (cr0));

    /*
    uint32_t cr3;
    asm volatile("mov %%cr3, %0": "=b" (cr3));

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
