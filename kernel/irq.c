#include <stddef.h>
#include <system.h>
#include <io.h>

#define NUM_IRQ_HANDLERS    16

/* These special IRQs point to the special IRQ handler
 * rather than the default 'fault_handler' */
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

typedef void (*irq_handler)(struct regs *r);

irq_handler irq_routines[NUM_IRQ_HANDLERS] = { 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

void irq_install_handler(int irq, void (*handler)(struct regs *r))
{
    if ((irq > NUM_IRQ_HANDLERS) && (irq >= 0)) {
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
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}

void irq_install(void)
{
    irq_remap();

    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
}

/*
 * Each IRQ ISR calls this handler (from irq_common_stub)
 * rather than the 'fault_handler' defined in 'isrs.c'.
 */
void default_irq_handler(struct regs *r)
{
    /* empty handler pointer */
    void (*handler)(struct regs *r);

    /* run custom handler if installed */
    int irq = r->int_no - 32;
    if ((irq < NUM_IRQ_HANDLERS) && (irq >= 0)) {
        handler = irq_routines[r->int_no - 32];
        if (handler) {
            handler(r);
        }
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
