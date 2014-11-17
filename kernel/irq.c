#include "assert.h"
#include "io.h"
#include "idt.h"
#include "x86.h"
#include "irq.h"


enum { IRQ_NUM_HANDLERS = 16 };
enum { IRQ_ISR_START = 32 };

/* These special IRQs point to the special IRQ handler
 * rather than the default 'fault_handler'
 *
 *  0        Programmable Interrupt Timer Interrupt
 *  1        Keyboard Interrupt
 *  2        Cascade (used internally by the two PICs. never raised)
 *  3        COM2 (if enabled)
 *  4        COM1 (if enabled)
 *  5        LPT2 (if enabled)
 *  6        Floppy Disk
 *  7        LPT1 / Unreliable "spurious" interrupt (usually)
 *  8        CMOS real-time clock (if enabled)
 *  9        Free for peripherals / legacy SCSI / NIC
 *  10       Free for peripherals / SCSI / NIC
 *  11       Free for peripherals / SCSI / NIC
 *  12       PS2 Mouse
 *  13       FPU / Coprocessor / Inter-processor
 *  14       Primary ATA Hard Disk
 *  15       Secondary ATA Hard Disk
 */

enum {
    MASTER_PIC_CMD = 0x20,
    MASTER_PIC_DATA = 0x21,
    SLAVE_PIC_CMD = 0xA0,
    SLAVE_PIC_DATA = 0xA1,
    PIC_EOI = 0x20
};

extern void irq0(), irq1(), irq2(), irq3(), irq4(), irq5(), irq6(), irq7();
extern void irq8(), irq9(), irq10(), irq11(), irq12(), irq13(), irq14(), irq15();

static int_handler_t g_isrs[IRQ_NUM_HANDLERS];

/* static copy of the IRQ mask from MASTER/SLAVE PIC data ports */
static uint16_t g_irq_mask;

static void update_irq_mask(uint16_t mask) {
    /* if the lower 8-bits don't match, update the MASTER port */
    if ((mask & 0xFF) != (g_irq_mask & 0xFF)) {
        outportb(MASTER_PIC_DATA, mask & 0xFF);
    }

    /* if the upper 8-bits don't match, update the SLAVE port */
    if (((mask >> 8) & 0xFF) != ((g_irq_mask >> 8) & 0xFF)) {
        outportb(SLAVE_PIC_DATA, (mask >> 8) & 0xFF);
    }
    /* update our static record of the irq mask */
    g_irq_mask = mask;
}

void enable_irq(unsigned int irq)
{
    bool iflag = beg_int_atomic();
    KASSERT(irq <= IRQ_NUM_HANDLERS);
    uint16_t mask = g_irq_mask;
    mask &= ~(1 << irq);
    update_irq_mask(mask);
    end_int_atomic(iflag);
}

void disable_irq(unsigned int irq)
{
    bool iflag = beg_int_atomic();
    KASSERT(irq <= IRQ_NUM_HANDLERS);
    uint16_t mask = g_irq_mask;
    mask |= 1 << irq;
    update_irq_mask(mask);
    end_int_atomic(iflag);
}

bool irq_enabled(unsigned int irq)
{
    bool iflag = beg_int_atomic();
    KASSERT(irq <= IRQ_NUM_HANDLERS);
    uint16_t mask = g_irq_mask;
    bool enabled = (mask & (1 << irq)) == 0;
    end_int_atomic(iflag);
    return enabled;
}

/* Normally, IRQ 0-8 are mapped to IDT entries 8-15.
 * These conflict with the IDT entries already installed.
 * The PIC(s) (8259) can be programmed to remap IRQ 0-15
 * to IDT entries 32-47
 */
static void irq_remap(void)
{
    outportb(MASTER_PIC_CMD, 0x11); /* write ICW1 to PICM, we are gonna write commands to PICM */
    outportb(SLAVE_PIC_CMD, 0x11); /* write ICW1 to PICS, we are gonna write commands to PICS */

    outportb(MASTER_PIC_DATA, IRQ_ISR_START); /* remap PICM to MASTER_PIC_CMD (32 decimal) */
    outportb(SLAVE_PIC_DATA, IRQ_ISR_START + 8); /* remap PICS to 0x28 (40 decimal) */

    outportb(MASTER_PIC_DATA, 0x04); /* IRQ2 -> connection to slave */
    outportb(SLAVE_PIC_DATA, 0x02);

    outportb(MASTER_PIC_DATA, 0x01); /* write ICW4 to PICM, we are gonna write commands to PICM */
    outportb(SLAVE_PIC_DATA, 0x01); /* write ICW4 to PICS, we are gonna write commands to PICS */

    /* disable (mask) all but 2 IRQs on PICM */
    outportb(MASTER_PIC_DATA, g_irq_mask & 0xFF);
    /* disable (mask) all IRQs on PICS */
    outportb(SLAVE_PIC_DATA, (g_irq_mask >> 8) & 0xFF);
}

void irq_install(void)
{
    /* IMPORTANT: initialize IRQ mask to 0xFFFB,
     * which disables all IRQs except the cascade (for slave PIC)
     */
    g_irq_mask = 0xFFFB;
    irq_remap();

    idt_set_int_gate(IRQ_ISR_START, (uintptr_t)irq0, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 1, (uintptr_t)irq1, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 2, (uintptr_t)irq2, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 3, (uintptr_t)irq3, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 4, (uintptr_t)irq4, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 5, (uintptr_t)irq5, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 6, (uintptr_t)irq6, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 7, (uintptr_t)irq7, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 8, (uintptr_t)irq8, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 9, (uintptr_t)irq9, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 10, (uintptr_t)irq10, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 11, (uintptr_t)irq11, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 12, (uintptr_t)irq12, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 13, (uintptr_t)irq13, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 14, (uintptr_t)irq14, KERNEL_DPL);
    idt_set_int_gate(IRQ_ISR_START + 15, (uintptr_t)irq15, KERNEL_DPL);
}

void irq_install_handler(unsigned int irq, int_handler_t handler)
{
    KASSERT(irq < IRQ_NUM_HANDLERS);
    g_isrs[irq] = handler;
}

/*
 * Each IRQ ISR calls this handler (from irq_common_stub),
 * which, in turn, will call IRQ-specific handlers
 */
void default_irq_handler(struct regs *r)
{
    /* empty handler pointer */
    int_handler_t handler = NULL;

    int irq = r->int_no - IRQ_ISR_START;
    KASSERT((irq < IRQ_NUM_HANDLERS) && (irq >= 0));

    /* run IRQ-specific handler if installed */
    handler = g_isrs[irq];
    if (handler != NULL) {
        handler(r);
    }

    /* if the IDT entry invoked is greater than 40
     * (meaning IRQ8-15), then send 'End of Interrupt' to
     * slave interrupt controller */
    if (irq > 7) {
        outportb(SLAVE_PIC_CMD, PIC_EOI);
    }

    /* regardless, send an EOI to master interrupt controller */
    outportb(MASTER_PIC_CMD, PIC_EOI);
}
