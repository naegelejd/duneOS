#include "system.h"
#include "multiboot.h"
#include "screen.h"
#include "print.h"
#include "kb.h"
#include "rtc.h"

int main(struct multiboot_info *mbinfo, multiboot_uint32_t mboot_magic)
{
    /* double check that interrupts are disabled */
    /* cli(); */
    k_clear_screen();

    if (mboot_magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("Multiboot Successful\n");
    } else {
        kprintf("Bad multiboot\n");
        return 1;
    }

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
    kprintf("Memory low: 0x%x\n", mbinfo->mem_lower);
    kprintf("Memory high: 0x%x\n", mbinfo->mem_upper);

    extern intptr_t start, code, data, bss, end;
    kprintf("Kernel size in bytes: %u\n", &end - &start);
    kprintf("Start: 0x%x\n", &start);
    kprintf("Code: 0x%x\n", &code);
    kprintf("Data: 0x%x\n", &data);
    kprintf("BSS: 0x%x\n", &bss);
    kprintf("End: 0x%x\n", &end);

    /*
    char buf[15];
    ksprintf(buf, "%s %d\n", "Hello", 42);
    kprintf(buf);

    kprintf("%04d\n", 42);
    kprintf("%04d\n", 12345);
    kprintf("%04x\n", 0xabcde);
    kprintf("%04X\n", 0xFE);
    kprintf("%4d\n", 42);
    kprintf("%4d\n", 12345);
    kprintf("%4X\n", 0xabcde);
    kprintf("%4x\n", 0xFE);
    */

    /*
    struct tm dt;
    while (1) {
        datetime(&dt);
        kprintf("%u:%u:%u %s %u, %u\n", dt.hour, dt.min, dt.sec,
                month_name(dt.month), dt.mday, dt.year);
        delay(2000);
    }
    */

    /* playing around */
    /*
    k_set_cursor(0);
    beep(2);
    delay(5000);
    reboot();
    */
    return 0;
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
