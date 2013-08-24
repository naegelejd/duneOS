#include "util.h"
#include "screen.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "mem.h"
#include "thread.h"
#include "tss.h"
#include "kb.h"
#include "rtc.h"
#include "timer.h"

extern uintptr_t g_start, g_code, g_data, g_bss, g_end;

void print_date(uint32_t arg)
{
    (void)arg;
    unsigned int row, col;
    struct tm dt;
    while (true) {
        sleep(50);
        datetime(&dt);
        kget_cursor(&row, &col);
        kset_cursor(0, 58);
        kprintf("%02u:%02u:%02u %s %02u, %04u\n", dt.hour, dt.min, dt.sec,
                month_name(dt.month), dt.mday, dt.year);
        kset_cursor(row, col);

        arg--;
    }
    exit(0);
}

void echo_input(uint32_t arg)
{
    (void)arg;
    while (true) {
        keycode_t kc = wait_for_key();
        kputc(kc);
    }
    exit(0);
}

void main(struct multiboot_info *mbinfo, multiboot_uint32_t mboot_magic)
{
    /* interrupts are disabled */

    kcls();
    kprintf("Welcome to DuneOS...\n\n");
    if (mboot_magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("Multiboot Successful\n");
    } else {
        kprintf("Bad multiboot\n");
        return;
    }

    bss_init();     /* zero all static data */
    gdt_install();
    kprintf("GDT installed\n");
    idt_install();
    kprintf("IDT and ISRs installed\n");
    irq_install();
    kprintf("IRQ handlers installed\n");
    tss_init();
    mem_init(mbinfo);
    kprintf("Memory manager and Heap initialized\n");

    /* re-enable interrupts */
    sti();

    scheduler_init();
    kprintf("Scheduler initialized\n");

    /* paging_install(&g_end); */
    /* kprintf("Paging enabled\n"); */

    char *tmp = "Hello World!\n";
    char *new = malloc(strlen(tmp) + 1);
    memcpy(new, tmp, strlen(tmp));
    new[strlen(tmp)] = '\0';
    kprintf("%s", new);
    free(new);

    timer_install();
    keyboard_install();
    rtc_install();

    /* start thread to print date/time on screen */
    thread_t* date_printer = start_kernel_thread(
            print_date, 0, PRIORITY_NORMAL, false);

    /* start thread to read keyboard input and echo it to screen */
    thread_t* echoer = start_kernel_thread(
            echo_input, 0, PRIORITY_NORMAL, false);

    /* wait for child threads to finish (forever) */
    join(echoer);
    join(date_printer);

    /* uint32_t* page_fault = 0xFFFFF0000; */
    /* kprintf("page fault? 0x%x\n", *page_fault); */
    /* kprintf("page fault? %u\n", *page_fault); */

    /* playing around */
    /*
    kprintf("%u\n", 1 / 0);

    beep(2);
    delay(5000);
    kreboot();
    */

    kcls();
    kprintf("Goodbye!");
}
