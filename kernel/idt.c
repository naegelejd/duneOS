#include "screen.h"
#include "string.h"
#include "x86.h"
#include "idt.h"


enum { IDT_NUM_ENTRIES = 256 };
enum { NUM_INT_EXCEPTIONS = 32 };

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct int_gate {
    uint16_t base_low;
    uint16_t sel;           /* kernel segment */
    unsigned reserved: 5;   /* set to 0 */
    unsigned sig: 8;        /* always 01110000b */
    unsigned dpl: 2;        /* Ring # (0-3) */
    unsigned present: 1;    /* segment present? */
    uint16_t base_high;
};

union idt_descr {
    struct int_gate intg;
    /* struct trap_gate trpg; */
    /* struct task_gate tskg; */
};

/* defined in start.s */
extern void isr0(), isr1(), isr2(), isr3(), isr4(), isr5(), isr6(), isr7();
extern void isr8(), isr9(), isr10(), isr11(), isr12(), isr13(), isr14(), isr15();
extern void isr16(), isr17(), isr18(), isr19(), isr20(), isr21(), isr22(), isr23();
extern void isr24(), isr25(), isr26(), isr27(), isr28(), isr29(), isr30(), isr31();
extern void isr128();

/* empty array of custom interrupt handlers */
static int_handler_t g_handlers[IDT_NUM_ENTRIES];

/* Our IDT has 256 entries. We only use the first 32,
 * but the rest serve as a safety net. When an undefined
 * IDT entry is encountered, or if the 'presence' bit is
 * cleared, a "Unhandled Interrupt" exception will be thrown.
 */
static union idt_descr g_idt[IDT_NUM_ENTRIES];
static struct idt_ptr g_idt_ptr;


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
    dbgprintf("Sizeof ISR with... no error code: %u, error code: %u\n",
            isr_no_err_size, isr_err_size);

    /* Set up the special IDT pointer */
    g_idt_ptr.limit = sizeof(union idt_descr) * IDT_NUM_ENTRIES;
    g_idt_ptr.base = (uint32_t)&g_idt;

    memset(&g_idt, 0, sizeof(union idt_descr) * IDT_NUM_ENTRIES);

    /* Install first 32 defined ISRs */
    idt_set_int_gate(0, (uintptr_t)isr0, KERNEL_DPL);
    idt_set_int_gate(1, (uintptr_t)isr1, KERNEL_DPL);
    idt_set_int_gate(2, (uintptr_t)isr2, KERNEL_DPL);
    idt_set_int_gate(3, (uintptr_t)isr3, KERNEL_DPL);
    idt_set_int_gate(4, (uintptr_t)isr4, KERNEL_DPL);
    idt_set_int_gate(5, (uintptr_t)isr5, KERNEL_DPL);
    idt_set_int_gate(6, (uintptr_t)isr6, KERNEL_DPL);
    idt_set_int_gate(7, (uintptr_t)isr7, KERNEL_DPL);
    idt_set_int_gate(8, (uintptr_t)isr8, KERNEL_DPL);
    idt_set_int_gate(9, (uintptr_t)isr9, KERNEL_DPL);
    idt_set_int_gate(10, (uintptr_t)isr10, KERNEL_DPL);
    idt_set_int_gate(11, (uintptr_t)isr11, KERNEL_DPL);
    idt_set_int_gate(12, (uintptr_t)isr12, KERNEL_DPL);
    idt_set_int_gate(13, (uintptr_t)isr13, KERNEL_DPL);
    idt_set_int_gate(14, (uintptr_t)isr14, KERNEL_DPL);
    idt_set_int_gate(15, (uintptr_t)isr15, KERNEL_DPL);
    idt_set_int_gate(16, (uintptr_t)isr16, KERNEL_DPL);
    idt_set_int_gate(17, (uintptr_t)isr17, KERNEL_DPL);
    idt_set_int_gate(18, (uintptr_t)isr18, KERNEL_DPL);
    idt_set_int_gate(19, (uintptr_t)isr19, KERNEL_DPL);
    idt_set_int_gate(20, (uintptr_t)isr20, KERNEL_DPL);
    idt_set_int_gate(21, (uintptr_t)isr21, KERNEL_DPL);
    idt_set_int_gate(22, (uintptr_t)isr22, KERNEL_DPL);
    idt_set_int_gate(23, (uintptr_t)isr23, KERNEL_DPL);
    idt_set_int_gate(24, (uintptr_t)isr24, KERNEL_DPL);
    idt_set_int_gate(25, (uintptr_t)isr25, KERNEL_DPL);
    idt_set_int_gate(26, (uintptr_t)isr26, KERNEL_DPL);
    idt_set_int_gate(27, (uintptr_t)isr27, KERNEL_DPL);
    idt_set_int_gate(28, (uintptr_t)isr28, KERNEL_DPL);
    idt_set_int_gate(29, (uintptr_t)isr29, KERNEL_DPL);
    idt_set_int_gate(30, (uintptr_t)isr30, KERNEL_DPL);
    idt_set_int_gate(31, (uintptr_t)isr31, KERNEL_DPL);

    /* only ring-3 interrupt - system calls */
    idt_set_int_gate(128, (uintptr_t)isr128, USERMODE_DPL);

    /* defined in 'start.s' */
    idt_flush(&g_idt_ptr);
}

void int_install_handler(int interrupt, int_handler_t handler)
{
    KASSERT((interrupt < IDT_NUM_ENTRIES) && (interrupt >= 0));
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
