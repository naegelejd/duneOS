#include "system.h"
#include "screen.h"
#include "print.h"
#include "kb.h"
#include "rtc.h"

void main() {
    /* double check that interrupts are disabled */
    cli();

    k_clear_screen();
    kprintf("Welcome to DuneOS...\n\n");
    
    gdt_install();
    kprintf("GDT installed\n");

    idt_install();
    kprintf("IDT installed\n");
    kprintf("ISRs installed\n");

    irq_install();
    timer_install();
    keyboard_install();
    rtc_install();
    kprintf("IRQ handlers installed\n");

    sti();


    /* debugging */
    extern intptr_t start, code, data, bss, end;
    kprintf("Kernel size in bytes: %u\n", &end - &start);
    kprintf("Start: 0x%x\n", &start);
    kprintf("Code: 0x%x\n", &code);
    kprintf("Data: 0x%x\n", &data);
    kprintf("BSS: 0x%x\n", &bss);
    kprintf("End: 0x%x\n", &end);


    kprintf("0x%08x\n", 0xABC);

    struct tm dt;
    while (1) {
        datetime(&dt);
        kprintf("%u:%u:%u %s %u, %u\n", dt.hour, dt.min, dt.sec,
                month_name(dt.month), dt.mday, dt.year);
        delay(2000);
    }

    /* playing around */
    /*
    k_set_cursor(0);
    beep(2);
    delay(5000);
    reboot();
    */
}

/* hack a hard reset by loading a bogus IDT */
uint32_t no_idt[2] = {0, 0};
void reboot()
{
    __asm__ ("lidt no_idt");
}

void halt()
{
    __asm__ ("cli\nhlt");
}

void cli()
{
    __asm__ ("cli");
}

void sti()
{
    __asm__("sti");
}
