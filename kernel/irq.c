#include <stddef.h>
#include <system.h>
#include <io.h>

#define NUM_IRQ_HANDLERS    16

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
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

irq_handler irq_routines[NUM_IRQ_HANDLERS] = { 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

void irq_install_handler(int irq, irq_handler handler)
{
    if ((irq < NUM_IRQ_HANDLERS) && (irq >= 0)) {
        irq_routines[irq] = handler;
    }
}

/* Normally, IRQ 0-8 are mapped to IDT entries 8-15.
 * These conflict with the IDT entries already installed.
 * The PIC(s) (8259) can be programmed to remap IRQ 0-15
 * to IDT entries 32-47
 */
void irq_remap(void)
{
    outportb(0x20, 0x11); /* write ICW1 to PICM, we are gonna write commands to PICM */
    outportb(0xA0, 0x11); /* write ICW1 to PICS, we are gonna write commands to PICS */

    outportb(0x21, 0x20); /* remap PICM to 0x20 (32 decimal) */
    outportb(0xA1, 0x28); /* remap PICS to 0x28 (40 decimal) */

    outportb(0x21, 0x04); /* IRQ2 -> connection to slave */ 
    outportb(0xA1, 0x02);

    outportb(0x21, 0x01); /* write ICW4 to PICM, we are gonna write commands to PICM */
    outportb(0xA1, 0x01); /* write ICW4 to PICS, we are gonna write commands to PICS */

    outportb(0x21, 0x0); /* enable all IRQs on PICM */
    outportb(0xA1, 0x0); /* enable all IRQs on PICS */
}

void irq_install(void)
{
    irq_remap();

    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

/*
 * Each IRQ ISR calls this handler (from irq_common_stub)
 * rather than the 'fault_handler' defined in 'isrs.c'.
 */
void default_irq_handler(struct regs *r)
{
    /* empty handler pointer */
    irq_handler handler;

    /* run custom handler if installed */
    int irq = r->int_no - 32;
    if ((irq < NUM_IRQ_HANDLERS) && (irq >= 0)) {
        handler = irq_routines[irq];
    } else {
        handler = NULL;
    }

    /* execute IRQ-specific handler if one exists */
    if (handler) {
        handler(r);
    }

    /* if the IDT entry invoked is greater than 40
     * (meaning IRQ8-15), then send 'End of Interrupt' to
     * slave interrupt controller */
    if (irq > 7) {
        outportb(0xA0, 0x20);
    }

    /* regardless, send and EOI to master interrupt controller */
    outportb(0x20, 0x20);
}
