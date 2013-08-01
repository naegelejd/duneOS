#include <stdint.h>

/*
 * GDT Access and Granularity fields
 *
 * Access bits:
 * 7: Segment Present? (1=Yes)
 * 6-5: Ring # (0-3)
 * 4: Descriptor type
 * 3-0: Type
 *
 * Granularity bits:
 * 7: Granularity (0 =1 byte, 1 = 4KB)
 * 6: Operand Size (0 = 16-bit, 1 = 32-bit)
 * 5: always 0
 * 4: always 0 (available for system)
 * 3-0: unrelated (Segment length bits 19:16)
 *
 * so... Access = 1001 1010
 * Granularity = 1100 1111
 */

/* entry in global descriptor table */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

/* special GDT pointer */
struct gdt_ptr {
    uint16_t limit;     /* max bytes used by GDT */
    uint32_t base;      /* base address of GDT */
} __attribute__((packed));

/* 3-entry GDT and special global GDT pointer */
struct gdt_entry gdt[3];
struct gdt_ptr gp;

/* defined in 'start.asm' */
extern void gdt_flush();

/* Set up a descriptor in GDT */
static void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit,
        uint8_t access, uint8_t gran)
{
    /* Set up descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Set up descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    /* Set up granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

/* called by main */
void gdt_install()
{
    /* Set up GDT pointer and limit */
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    /* NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Code segment
     * Granularity: 4KB
     * Base addr: 0
     * Limit: 4GB
     */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Data segment
     * same as Code segment
     */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Flush out the old GDT and install new */
    gdt_flush();
}
