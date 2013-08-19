#ifndef DUNE_GDT_H
#define DUNE_GDT_H

#include "dune.h"
#include "seg.h"

/*
 * Segment Descriptor
 *
 * Notes on Direction/Conforming (DC) bit:
 * Code segment:
 *   1: code in this segment can be executed in lower privilege levels
 *   0: code in this segment can only be accessed by Ring # in descriptor
 * Data segment:
 *   1: segment grows down
 *   0: segment grows up
 */
struct seg_descr {
    uint16_t limit_low;

    uint16_t base_low;
    uint8_t base_middle;

    /* access byte - starting with type nibble */
    unsigned accessed:1;    /* toggled by CPU */
    unsigned rw: 1;         /* code: readable? data: writable? tss: busy? */
    unsigned dc: 1;         /* direction/conforming */
    unsigned exec: 1;       /* executable segment? */

    unsigned system: 1;     /* 1 = code/data segment, 0 = system */

    unsigned dpl: 2;        /* descriptor privilege level (Ring 0-3) */
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


struct seg_descr* new_seg_descr(void);
uint16_t gdt_selector(struct seg_descr*);
uint8_t seg_descr_type(struct seg_descr*);
uint8_t seg_descr_access(struct seg_descr*);

void gdt_install();

#endif /* DUNE_GDT_H */
