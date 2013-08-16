#include "system.h"
#include "string.h"
#include "gdt.h"
#include "tss.h"


static struct tss g_tss;

void init_tss_seg_descr(struct seg_descr* descr, struct tss* tss)
{
    uintptr_t base = (uintptr_t)tss;
    uint32_t limit = sizeof(struct tss);

    descr->base_low = (base & 0xFFFF);
    descr->base_middle = (base >> 16) & 0xFF;
    descr->base_high = (base >> 24) & 0xFF;

    descr->limit_low = (limit & 0xFFFF);

    /* the type nibble is 0x9 (32-bit available TSS) */
    descr->accessed = 1;
    descr->rw = 0;          /* busy bit - NOT BUSY! on init */
    descr->dc = 0;
    descr->exec = 1;

    descr->system = 0;      /* tss seg, not code/data */

    descr->dpl = 0;         /* Ring 0 */
    descr->present = 1;

    descr->limit_high = (limit >> 16) & 0x0F;

    descr->avail = 0;        /* no longer available */
    descr->reserved = 0;
    descr->opsize = 0;      /* must be 0 in TSS */
    descr->granularity = 0; /* 1-byte granularity! */
}

void tss_init(void)
{
    struct seg_descr* tss_descr = new_seg_descr();

    memset(&g_tss, 0, sizeof(struct tss));
    init_tss_seg_descr(tss_descr, &g_tss);

    KASSERT(seg_descr_type(tss_descr) == 0x89);
    KASSERT(seg_descr_access(tss_descr) == 0x00);

    /* I know this is descriptor 3 in the GDT
     * 0: null descriptor
     * 1: ring 0 code descriptor
     * 2: ring 0 data descriptor
     * 3: TSS descriptor
     * so the TSS selector is 3 * (8 byte segment descriptor) = 0x18
     */
    uint16_t tss_sel = gdt_selector(tss_descr);
    KASSERT(tss_sel = 0x18);

    /* load the TSS selector */
    asm volatile("ltr %0" : : "a" (tss_sel));
}
