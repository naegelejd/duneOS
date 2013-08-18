#include "screen.h"
#include "string.h"
#include "idt.h"


/* defined in start.s */
extern void isr0(), isr1(), isr2(), isr3(), isr4(), isr5(), isr6(), isr7();
extern void isr8(), isr9(), isr10(), isr11(), isr12(), isr13(), isr14(), isr15();
extern void isr16(), isr17(), isr18(), isr19(), isr20(), isr21(), isr22(), isr23();
extern void isr24(), isr25(), isr26(), isr27(), isr28(), isr29(), isr30(), isr31();
extern void isr128();

static char *exception_messages[] = {
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

/* empty array of custom interrupt handlers */
static int_handler_t g_handlers[NUM_IDT_ENTRIES];

/* Our IDT has 256 entries. We only use the first 32,
 * but the rest serve as a safety net. When an undefined
 * IDT entry is encountered, or if the 'presence' bit is
 * cleared, a "Unhandled Interrupt" exception will be thrown.
 */
static union idt_descr g_idt[NUM_IDT_ENTRIES];
static struct idt_ptr g_idt_ptr;


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

/* defined in assembly */
extern void idt_flush(void*);

/* install the IDT */
void idt_install()
{
    KASSERT(sizeof(union idt_descr) == sizeof(struct int_gate));

    /* TODO: calculate offsets to each ISR then loop over
     * each ISR address and add them to IDT... rather than
     * explicitly referencing each ISR symbol */
    char *isr7addr = (char*)(uintptr_t)isr7;    /* pedantic */
    char *isr8addr = (char*)(uintptr_t)isr8;    /* pedantic */
    char *isr9addr = (char*)(uintptr_t)isr9;    /* pedantic */
    uintptr_t isr_no_err_size = isr8addr - isr7addr;
    uintptr_t isr_err_size = isr9addr - isr8addr;
    kprintf("Sizeof ISR with... no error code: %u, error code: %u\n",
            isr_no_err_size, isr_err_size);

    /* Set up the special IDT pointer */
    g_idt_ptr.limit = sizeof(union idt_descr) * NUM_IDT_ENTRIES;
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

    idt_set_int_gate(128, (uintptr_t)isr128, 3);


    /* defined in 'start.s' */
    idt_flush(&g_idt_ptr);
}

void int_install_handler(int interrupt, int_handler_t handler)
{
    KASSERT((interrupt < NUM_IDT_ENTRIES) && (interrupt >= 0));
    g_handlers[interrupt] = handler;
}
/* Each interrupt handler calls this function (assembly),
 * which, in turn, either calls a specific handler, or
 * just prints an exception message and halts the CPU
 */
void default_int_handler(struct regs *r)
{
    int_handler_t handler = g_handlers[r->int_no];
    if (!handler) {
        if (r->int_no < NUM_INT_EXCEPTIONS) {
            /* Display the description for the exception */
            kset_attr(LGREEN, BLACK);
            kprintf("%s Exception. System Frozen!\n", exception_messages[r->int_no]);
        } else {
            kset_attr(MAGENTA, BLACK);
            kprintf("Unknown Interrupt 0x%x. System Frozen!\n", r->int_no);
        }
        khalt();
    } else {
        handler(r);
    }
}
