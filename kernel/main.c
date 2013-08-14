#include "system.h"
#include "multiboot.h"
#include "screen.h"
#include "print.h"
#include "rtc.h"

extern uintptr_t g_start, g_code, g_data, g_bss, g_end;

int main(struct multiboot_info *mbinfo, multiboot_uint32_t mboot_magic)
{
    /* double check that interrupts are disabled */
    /* cli(); */

    kcls();
    kprintf("Welcome to DuneOS...\n\n");
    if (mboot_magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("Multiboot Successful\n");
    } else {
        kprintf("Bad multiboot\n");
        return 1;
    }

    kprintf("Memory Map:\n");
    multiboot_memory_map_t *mmap = (multiboot_memory_map_t*)mbinfo->mmap_addr;
    while ((uintptr_t)mmap < mbinfo->mmap_addr + mbinfo->mmap_length) {
        mmap = (multiboot_memory_map_t*) ((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
        kprintf("Base Addr: 0x%x, Length: 0x%x, Available: %s\n", (uint32_t)mmap->addr, (uint32_t)mmap->len,
                (mmap->type == 1 ? "true" : "false"));
    }


    if (mbinfo->flags & MULTIBOOT_INFO_MEMORY) {
        kprintf("Memory low: 0x%x\n", mbinfo->mem_lower);
        kprintf("Memory high: 0x%x\n", mbinfo->mem_upper);
    } else {
        kprintf("No useful memory limits found\n");
    }

    gdt_install();
    kprintf("GDT installed\n");
    idt_install();
    kprintf("IDT and ISRs installed\n");
    irq_install();
    kprintf("IRQ handlers installed\n");

    mem_init(&g_end, mbinfo->mem_upper);

    paging_install(&g_end);
    kprintf("Paging enabled\n");

    timer_install();
    keyboard_install();
    rtc_install();

    sti();

    /* uint32_t* page_fault = 0xFFFFF0000; */
    /* kprintf("page fault? 0x%x\n", *page_fault); */
    /* kprintf("page fault? %u\n", *page_fault); */

    /*
    kprintf("Kernel size in bytes: %u\n", &g_end - &g_start);
    kprintf("Start: 0x%x\n", &g_start);
    kprintf("Code: 0x%x\n", &g_code);
    kprintf("Data: 0x%x\n", &g_data);
    kprintf("BSS: 0x%x\n", &g_bss);
    kprintf("End: 0x%x\n", &g_end);
    */

    struct tm dt;
    while (1) {
        delay(500);
        datetime(&dt);
        kset_cursor(0, 58);
        kprintf("%02u:%02u:%02u %s %02u, %04u\n", dt.hour, dt.min, dt.sec,
                month_name(dt.month), dt.mday, dt.year);
    }

    /* playing around */
    /*
    kprintf("%u\n", 1 / 0);

    beep(2);
    delay(5000);
    reboot();
    */
    return 0;
}
