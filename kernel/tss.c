#include "system.h"


struct tss {
    uint16_t link;
    uint16_t reserved0;

    /* stack pointers/selectors */
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved3;

    /* page directory reg */
    uint32_t cr3;

    /* general purpose regs */
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    /* segment selectors */
    uint16_t es;
    uint16_t reserved4;
    uint16_t cs;
    uint16_t reserved5;
    uint16_t ss;
    uint16_t reserved6;
    uint16_t ds;
    uint16_t reserved7;
    uint16_t fs;
    uint16_t reserved8;
    uint16_t gs;
    uint16_t reserved9;

    /* GDT selector for the LDT descriptor */
    uint16_t ldt;
    uint16_t reserved10;

    /* unsigned debugTrap : 1; */
    /* unsigned reserved11 : 15; */
    uint16_t reserved11;

    /* pointer to IO port permissions map for this task */
    uint16_t io_map_ptr;
};

void tss_init(void)
{

}
