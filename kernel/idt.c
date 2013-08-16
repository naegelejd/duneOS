#include "system.h"
#include "screen.h"
#include "string.h"
#include "idt.h"


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

static char *exception_messages[] =
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

/* Our IDT has 256 entries. We only use the first 32,
 * but the rest serve as a safety net. When an undefined
 * IDT entry is encountered, or if the 'presence' bit is
 * cleared, a "Unhandled Interrupt" exception will be thrown.
 */
union idt_descr g_idt[NUM_IDT_ENTRIES];
struct idt_ptr g_idt_ptr;


void fault_handler(struct regs *r)
{
    /* Check if the fault number is between 0-31 */
    if (r->int_no < 32) {
        /* Display the description for the exception */
        kset_attr(LGREEN, BLACK);
        kprintf("%s Exception. System Frozen!\n", exception_messages[r->int_no]);
        khalt();
    }
}

void idt_set_int_gate(uint8_t num, uintptr_t base, unsigned dpl)
{
    KASSERT((uintptr_t)&g_idt[num].intg == (uintptr_t)&g_idt[num]);
    struct int_gate* intg = &(g_idt[num].intg);

    intg->base_low = base & 0xFFFF;
    intg->sel = CODE_SEG_SELECTOR;
    intg->reserved = 0;
    intg->sig = 0x70;    /* always */
    intg->dpl = dpl;     /* descriptor protection level */
    intg->present = 1;
    intg->base_high = (base >> 16) & 0xFFFF;
}

/* defined in start.s */
extern void load_idt();

/* install the IDT */
void idt_install()
{
    KASSERT(sizeof(union idt_descr) == sizeof(struct int_gate));

    /* Set up the special IDT pointer */
    g_idt_ptr.limit = sizeof(union idt_descr) * NUM_IDT_ENTRIES - 1;
    g_idt_ptr.base = (uint32_t)&g_idt;

    memset(&g_idt, 0, sizeof(union idt_descr) * NUM_IDT_ENTRIES);

    /* Install first 32 defined ISRs */
    idt_set_int_gate(0, (uintptr_t)isr0, 0);
    idt_set_int_gate(1, (uintptr_t)isr1, 0);
    idt_set_int_gate(2, (uintptr_t)isr2, 0);
    idt_set_int_gate(3, (uintptr_t)isr3, 0);
    idt_set_int_gate(4, (uintptr_t)isr4, 0);
    idt_set_int_gate(5, (uintptr_t)isr5, 0);
    idt_set_int_gate(6, (uintptr_t)isr6, 0);
    idt_set_int_gate(7, (uintptr_t)isr7, 0);
    idt_set_int_gate(8, (uintptr_t)isr8, 0);
    idt_set_int_gate(9, (uintptr_t)isr9, 0);
    idt_set_int_gate(10, (uintptr_t)isr10, 0);
    idt_set_int_gate(11, (uintptr_t)isr11, 0);
    idt_set_int_gate(12, (uintptr_t)isr12, 0);
    idt_set_int_gate(13, (uintptr_t)isr13, 0);
    idt_set_int_gate(14, (uintptr_t)isr14, 0);
    idt_set_int_gate(15, (uintptr_t)isr15, 0);
    idt_set_int_gate(16, (uintptr_t)isr16, 0);
    idt_set_int_gate(17, (uintptr_t)isr17, 0);
    idt_set_int_gate(18, (uintptr_t)isr18, 0);
    idt_set_int_gate(19, (uintptr_t)isr19, 0);
    idt_set_int_gate(20, (uintptr_t)isr20, 0);
    idt_set_int_gate(21, (uintptr_t)isr21, 0);
    idt_set_int_gate(22, (uintptr_t)isr22, 0);
    idt_set_int_gate(23, (uintptr_t)isr23, 0);
    idt_set_int_gate(24, (uintptr_t)isr24, 0);
    idt_set_int_gate(25, (uintptr_t)isr25, 0);
    idt_set_int_gate(26, (uintptr_t)isr26, 0);
    idt_set_int_gate(27, (uintptr_t)isr27, 0);
    idt_set_int_gate(28, (uintptr_t)isr28, 0);
    idt_set_int_gate(29, (uintptr_t)isr29, 0);
    idt_set_int_gate(30, (uintptr_t)isr30, 0);
    idt_set_int_gate(31, (uintptr_t)isr31, 0);

    /* Add new ISRs to the IDT here */

    /* defined in 'start.s' */
    load_idt();
}
