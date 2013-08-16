#ifndef DUNE_SYSTEM_H
#define DUNE_SYSTEM_H

#include "dune.h"
#include "int.h"
#include "multiboot.h"


enum {
    IRQ_TIMER = 0,
    IRQ_KEYBOARD = 1,
    IRQ_RTC = 8
};

struct regs
{
    uint32_t gs, fs, es, ds;                /* pushed segs last */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    uint32_t int_no, err_code;              /* pushed in ISR */
    uint32_t epi, cs, eflags, useresp, ss;  /* pushed by proc automatically */
};

typedef void (*irq_handler)(struct regs *r);


void kreboot();
void khalt();

void gdt_install();
void tss_init(void);

void idt_install();
void idt_set_int_gate(uint8_t num, uintptr_t base, unsigned dpl);

void irq_install();
void irq_install_handler(int irq, irq_handler handler);

void bss_init(void);
void mem_init(struct multiboot_info*);

void paging_install(uintptr_t end);

void timer_install();
void delay(unsigned int ticks);
void set_timer_frequency(unsigned int hz);

void beep(unsigned int ticks);
void set_speaker_frequency(unsigned int hz);

void keyboard_install();

void rtc_install(void);

#endif /* DUNE_SYSTEM_H */
