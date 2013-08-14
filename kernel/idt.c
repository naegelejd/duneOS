#include <stdint.h>
#include <system.h>
#include "screen.h"
#include "string.h"
#include "print.h"

/*
 * IDT flag bits:
 *
 * 7: Segment is present (1=Yes)
 * 6-5: Which Ring (0-3)
 * 4-0: Always 01110 (0x0E)
 */

/* entry in IDT */
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;       /* kernel segment goes here */
    uint8_t always0;    /* always set to 0 */
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Our IDT has 256 entries. We only use the first 32,
 * but the rest serve as a safety net. When an undefined
 * IDT entry is encountered, or if the 'presence' bit is
 * cleared, a "Unhandled Interrupt" exception will be thrown.
 */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* defined in start.s */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void fault_handler(struct regs *r)
{
    /* Check if the fault number is between 0-31 */
    if (r->int_no < 32) {
        /* Display the description for the exception */
        kset_attr(LGREEN, BLACK);
        kprintf("%s Exception. System Frozen!\n", exception_messages[r->int_no]);
        halt();
    }
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].base_low = base & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].base_high = (base >> 16) & 0xFFFF;
}

/* defined in start.s */
extern void load_idt();

/* install the IDT */
void idt_install()
{
    /* Set up the special IDT pointer */
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    uint8_t sel = 0x08;     /* code segment */
    uint8_t flags = 1 << 7 | 0 << 5 | 0xE;  /* present, ring 0 (supervisor), 0xE */
    /* Install first 32 defined ISRs */
    idt_set_gate(0, (uint32_t)isr0, sel, flags);
    idt_set_gate(1, (uint32_t)isr1, sel, flags);
    idt_set_gate(2, (uint32_t)isr2, sel, flags);
    idt_set_gate(3, (uint32_t)isr3, sel, flags);
    idt_set_gate(4, (uint32_t)isr4, sel, flags);
    idt_set_gate(5, (uint32_t)isr5, sel, flags);
    idt_set_gate(6, (uint32_t)isr6, sel, flags);
    idt_set_gate(7, (uint32_t)isr7, sel, flags);
    idt_set_gate(8, (uint32_t)isr8, sel, flags);
    idt_set_gate(9, (uint32_t)isr9, sel, flags);
    idt_set_gate(10, (uint32_t)isr10, sel, flags);
    idt_set_gate(11, (uint32_t)isr11, sel, flags);
    idt_set_gate(12, (uint32_t)isr12, sel, flags);
    idt_set_gate(13, (uint32_t)isr13, sel, flags);
    idt_set_gate(14, (uint32_t)isr14, sel, flags);
    idt_set_gate(15, (uint32_t)isr15, sel, flags);
    idt_set_gate(16, (uint32_t)isr16, sel, flags);
    idt_set_gate(17, (uint32_t)isr17, sel, flags);
    idt_set_gate(18, (uint32_t)isr18, sel, flags);
    idt_set_gate(19, (uint32_t)isr19, sel, flags);
    idt_set_gate(20, (uint32_t)isr20, sel, flags);
    idt_set_gate(21, (uint32_t)isr21, sel, flags);
    idt_set_gate(22, (uint32_t)isr22, sel, flags);
    idt_set_gate(23, (uint32_t)isr23, sel, flags);
    idt_set_gate(24, (uint32_t)isr24, sel, flags);
    idt_set_gate(25, (uint32_t)isr25, sel, flags);
    idt_set_gate(26, (uint32_t)isr26, sel, flags);
    idt_set_gate(27, (uint32_t)isr27, sel, flags);
    idt_set_gate(28, (uint32_t)isr28, sel, flags);
    idt_set_gate(29, (uint32_t)isr29, sel, flags);
    idt_set_gate(30, (uint32_t)isr30, sel, flags);
    idt_set_gate(31, (uint32_t)isr31, sel, flags);

    /* Add new ISRs to the IDT here */

    /* defined in 'start.s' */
    load_idt();
}
