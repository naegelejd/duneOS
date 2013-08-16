#include "system.h"
#include "string.h"

/*
 * Segment Descriptor
 *
 * Notes on Direction/Conforming (DC) bit:
 * Code segment:
 *   1: code in this segment can be executed in lower privelege levels
 *   0: code in this segment can only be accessed by ring # in descriptor
 * Data segment:
 *   1: segment grows down
 *   0: segment grows up
 */
struct seg_descr {
    uint16_t limit_low;

    uint16_t base_low;
    uint8_t base_middle;

    /* access byte */
    unsigned accessed:1;    /* toggled by CPU */
    unsigned rw: 1;         /* code: readable? data: writable? */
    unsigned dc: 1;         /* direction/conforming */
    unsigned exec: 1;       /* executable segment? */
    unsigned system: 1;     /* always 1 */
    unsigned ring: 2;       /* Ring 0-3 */
    unsigned present: 1;    /* present? 1=Yes */

    unsigned limit_high: 4;

    /* flags nibble */
    unsigned avail: 1;      /* populated? 0=no, 1=yes */
    unsigned reserved: 1;   /* always 0 */
    unsigned opsize: 1;     /* 0=16-bit, 1=32-bit */
    unsigned granularity: 1;/* 0=1 byte, 1=4KB */

    uint8_t base_high;
} __attribute__((packed));


enum { NUM_GDT_ENTRIES = 8 };

/* special GDT pointer */
struct gdt_ptr {
    uint16_t limit;     /* max bytes used by GDT */
    uint32_t base;      /* base address of GDT */
} __attribute__((packed));

/* GDT and special global GDT pointer */
struct seg_descr g_gdt[NUM_GDT_ENTRIES];
struct gdt_ptr g_gdt_ptr;

/* defined in 'start.asm' */
extern void gdt_flush();

void init_code_seg_descr(struct seg_descr *descr, uintptr_t base, uintptr_t limit, unsigned ring)
{
    descr->base_low = (base & 0xFFFF);
    descr->base_middle = (base >> 16) & 0xFF;
    descr->base_high = (base >> 24) & 0xFF;

    descr->limit_low = (limit & 0xFFFF);
    descr->limit_high = (limit >> 16) & 0x0F;

    descr->present =  1;
    descr->ring = ring;
    descr->system = 1;

    descr->accessed = 0;
    descr->rw = 1;          /* readable */
    descr->dc = 0;          /* only executable in ring 0 */
    descr->exec = 1;        /* executable */

    descr->granularity = 1;
    descr->opsize = 1;
    descr->reserved = 0;
    descr->avail = 0;
}

void init_data_seg_descr(struct seg_descr *descr, uintptr_t base, uintptr_t limit, unsigned ring)
{
    descr->base_low = (base & 0xFFFF);
    descr->base_middle = (base >> 16) & 0xFF;
    descr->base_high = (base >> 24) & 0xFF;

    descr->limit_low = (limit & 0xFFFF);
    descr->limit_high = (limit >> 16) & 0xFF;

    descr->present =  1;
    descr->ring = ring;
    descr->system = 1;

    descr->accessed = 0;
    descr->rw = 1;          /* writable */
    descr->dc = 0;          /* only executable in ring 0 */
    descr->exec = 0;        /* NOT executable */

    descr->granularity = 1;
    descr->opsize = 1;
    descr->reserved = 0;
    descr->avail = 0;
}

void gdt_install()
{
    KASSERT(sizeof(struct seg_descr) == 8);

    /* Set up GDT pointer and limit */
    g_gdt_ptr.limit = sizeof(struct seg_descr) * NUM_GDT_ENTRIES;
    g_gdt_ptr.base = (uint32_t)&g_gdt;

    /* NULL descriptor */
    memset(&g_gdt[0], 0, sizeof(struct seg_descr));

    /* Code segment, Base addr: 0, Limit: 4GB */
    init_code_seg_descr(&g_gdt[1], 0, 0xFFFFFFFF, 0);

    uint8_t *tmp = (uint8_t*)&g_gdt[1] + 5;
    KASSERT(*tmp == 0x9A);
    KASSERT(*(tmp+1) == 0xCF);

    /* Data segment, Base addr: 0, Limit: 4GB */
    init_data_seg_descr(&g_gdt[2], 0, 0xFFFFFFFF, 0);

    tmp = (uint8_t*)&g_gdt[2] + 5;
    KASSERT(*tmp == 0x92);
    KASSERT(*(tmp+1) == 0xCF);

    /* Flush out the old GDT and install new */
    gdt_flush();
}
