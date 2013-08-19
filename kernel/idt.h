#ifndef DUNE_IDT_H
#define DUNE_IDT_H

#include "int.h"
#include "seg.h"    /* for selector constants */

struct int_gate {
    uint16_t base_low;
    uint16_t sel;           /* kernel segment */
    unsigned reserved: 5;   /* set to 0 */
    unsigned sig: 8;        /* always 01110000b */
    unsigned dpl: 2;        /* Ring # (0-3) */
    unsigned present: 1;    /* segment present? */
    uint16_t base_high;
} __attribute__((packed));

union idt_descr {
    struct int_gate intg;
    /* struct trap_gate trpg; */
    /* struct task_gate tskg; */
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


enum { NUM_IDT_ENTRIES = 256 };
enum { NUM_INT_EXCEPTIONS = 32 };


void idt_install();
void idt_set_int_gate(uint8_t num, uintptr_t base, unsigned dpl);


#endif /* DUNE_IDT_H */
