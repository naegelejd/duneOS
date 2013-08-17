#include "string.h"
#include "gdt.h"

/* GDT and special global GDT pointer */
static struct seg_descr g_gdt[NUM_GDT_ENTRIES];
static unsigned int g_num_gdt_entries = 0;

struct gdt_ptr g_gdt_ptr;


struct seg_descr* new_seg_descr(void)
{
    KASSERT(g_num_gdt_entries < NUM_GDT_ENTRIES);

    struct seg_descr* sd = &g_gdt[g_num_gdt_entries];
    g_num_gdt_entries++;
    return sd;
}

uint16_t gdt_selector(struct seg_descr* sd)
{
    /*
     * (# of segment descriptors from GDT start)
     *                    *
     *   (sizeof segment descriptor in bytes)
     */
    return (sd - &g_gdt[0]) * sizeof(struct seg_descr);
}

uint8_t seg_descr_type(struct seg_descr* sd)
{
    uint8_t type = *((uint8_t*)sd + 5);
    return type;
}

uint8_t seg_descr_access(struct seg_descr* sd)
{
    uint8_t access = *((uint8_t*)sd + 6);
    return access;
}

void init_code_seg_descr(struct seg_descr *descr, uintptr_t base, uintptr_t limit, unsigned dpl)
{
    descr->base_low = (base & 0xFFFF);
    descr->base_middle = (base >> 16) & 0xFF;
    descr->base_high = (base >> 24) & 0xFF;

    descr->limit_low = (limit & 0xFFFF);

    descr->accessed = 0;
    descr->rw = 1;          /* readable */
    descr->dc = 0;          /* non-conforming (only executable in Ring 0) */
    descr->exec = 1;        /* executable */

    descr->system = 1;      /* code seg, not TSS */

    descr->dpl = dpl;
    descr->present =  1;

    descr->limit_high = (limit >> 16) & 0x0F;

    descr->avail = 0;       /* no longer available */
    descr->reserved = 0;
    descr->opsize = 1;      /* 32-bit operands */
    descr->granularity = 1; /* 4KB */
}

void init_data_seg_descr(struct seg_descr *descr, uintptr_t base, uintptr_t limit, unsigned dpl)
{
    descr->base_low = (base & 0xFFFF);
    descr->base_middle = (base >> 16) & 0xFF;
    descr->base_high = (base >> 24) & 0xFF;

    descr->limit_low = (limit & 0xFFFF);
    descr->limit_high = (limit >> 16) & 0xFF;

    descr->accessed = 0;
    descr->rw = 1;          /* writable */
    descr->dc = 0;          /* segment grows UP */
    descr->exec = 0;        /* NOT executable */

    descr->system = 1;      /* data seg, not TSS */

    descr->dpl = dpl;
    descr->present =  1;

    descr->avail = 0;       /* no longer available */
    descr->reserved = 0;
    descr->opsize = 1;      /* 32-bit operands */
    descr->granularity = 1; /* 4KB granularity */
}

/* defined in 'start.asm' */
extern void gdt_flush(void*);

void gdt_install()
{
    KASSERT(sizeof(struct seg_descr) == 8);

    /* Set up GDT pointer and limit */
    g_gdt_ptr.limit = sizeof(struct seg_descr) * NUM_GDT_ENTRIES;
    g_gdt_ptr.base = (uint32_t)&g_gdt;

    /* NULL descriptor */
    struct seg_descr* null_descr = new_seg_descr();
    KASSERT(gdt_selector(null_descr) == NULL_SEG_SELECTOR);
    memset(null_descr, 0, sizeof(struct seg_descr));

    /* Code segment, Base addr: 0, Limit: 2^20 - 1 4KB pages */
    struct seg_descr* cs_descr = new_seg_descr();
    KASSERT(gdt_selector(cs_descr) == CODE_SEG_SELECTOR);
    init_code_seg_descr(cs_descr, 0, 0xFFFFF, 0);

    KASSERT(seg_descr_type(&g_gdt[1]) == 0x9A);
    KASSERT(seg_descr_access(&g_gdt[1]) == 0xCF);

    /* Data segment, Base addr: 0, Limit: 2^20 - 1 4KB pages */
    struct seg_descr* ds_descr = new_seg_descr();
    KASSERT(gdt_selector(ds_descr) == DATA_SEG_SELECTOR);
    init_data_seg_descr(ds_descr, 0, 0xFFFFF, 0);

    KASSERT(seg_descr_type(&g_gdt[2]) == 0x92);
    KASSERT(seg_descr_access(&g_gdt[2]) == 0xCF);

    /* Flush out the old GDT and install new */
    gdt_flush(&g_gdt_ptr);
}
