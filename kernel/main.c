#include "util.h"
#include "screen.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "mem.h"
#include "spkr.h"
#include "paging.h"
#include "syscall.h"
#include "thread.h"
#include "kb.h"
#include "rtc.h"
#include "timer.h"
#include "mouse.h"
#include "blkdev.h"
#include "initrd.h"

extern uintptr_t g_start, g_code, g_data, g_bss, g_end;

static void print_date(uint32_t arg);
static void echo_input(uint32_t arg);
static void hog_cpu(uint32_t arg);
static void usermode_test(uint32_t arg);

void stat_mouse(uint32_t arg)
{
    (void)arg;
    unsigned int row, col;
    extern int8_t g_mouse_dx, g_mouse_dy;
    extern bool g_mouse_left, g_mouse_right;
    while (true) {
        kget_cursor(&row, &col);
        kset_cursor(24, 64);
        kprintf("%02d,%02d", g_mouse_dx, g_mouse_dy);
        if (g_mouse_left) {
            kset_cursor(24, 70);
            kprintf("l");
        }
        if (g_mouse_right) {
            kset_cursor(24, 72);
            kprintf("r");
        }
        kset_cursor(row, col);
        sleep(200);
    }
}


struct modinfo {
    uintptr_t start;
    uintptr_t end;
};

uintptr_t load_mods(struct multiboot_info *mbinfo, struct modinfo *initrd_info)
{
    uintptr_t start = 0, end = 0;
    if (mbinfo->mods_count > 0) {
        multiboot_module_t *mod = (multiboot_module_t*)phys_to_virt(mbinfo->mods_addr);
        initrd_info->start = phys_to_virt(mod->mod_start);
        initrd_info->end = phys_to_virt(mod->mod_end);

        unsigned int i;
        for (i = 0; i < mbinfo->mods_count; i++) {
            start = phys_to_virt(mod->mod_start);
            end = phys_to_virt(mod->mod_end);
            DEBUGF("Mod start: 0x%x, Mod end: 0x%x, Cmd line: %s\n",
                    start, end, (char*)phys_to_virt(mod->cmdline));
            mod++;
        }
    }
    return end;
}

void kmain(struct multiboot_info *mbinfo, multiboot_uint32_t mboot_magic)
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

    struct modinfo initrd_info;
    uintptr_t modsend = load_mods(mbinfo, &initrd_info);
    uintptr_t kend = (uintptr_t)(&g_end), kstart = (uintptr_t)(&g_start);
    kend = (modsend > kend) ? modsend : kend;
    kend = mem_init(mbinfo, kstart, kend);
    kprintf("Memory manager and Heap initialized\n");

    /* re-enable interrupts */
    sti();

    timer_install();
    keyboard_install();
    rtc_install();
    mouse_install();
    syscalls_install();

    scheduler_init();
    kprintf("Scheduler initialized\n");

    /* if (initrd_info.start || initrd_info.end) { */
    /*     size_t len = (char*)initrd_info.end - (char*)initrd_info.start; */
    /*     kprintf("Initializing RAMDisk @ 0x%x (%u bytes)\n", */
    /*             initrd_info.start, len); */
    /*     block_device_t* initrd = ramdisk_init(initrd_info.start, len); */
    /*     (void)initrd; */
    /* } */

    paging_install();
    kprintf("Paging enabled\n");

    char *tmp = "Hello World!\n";
    char *new = malloc(strlen(tmp) + 1);
    memcpy(new, tmp, strlen(tmp));
    new[strlen(tmp)] = '\0';
    kprintf("%s", new);
    free(new);

    DEBUGF("ESP: %X\n", get_esp());

    /* Run GRUB module (which just returns the value of register ESP */
    /* unsigned int len = initrd_info.end - initrd_info.start; */
    /* dumpmem(initrd_info.start, len); */
    /* typedef uint32_t (*call_module_t)(void); */
    /* void *cp = malloc(len); */
    /* memcpy(cp, initrd_info.start, len); */
    /* call_module_t mod0 = (call_module_t)cp; */
    /* uint32_t esp = mod0(); */
    /* kprintf("ESP: 0x%X\n", esp); */

    /* Test interrupt handler(s) */
    /* asm volatile ("int $0x03"); */

    /* Test ISR handler with a page fault */
    /* uintptr_t* page_fault = (uintptr_t*)0xFFFFF0000; */
    /* kprintf("page fault? 0x%x\n", *page_fault); */
    /* kprintf("page fault? %u\n", *page_fault); */

    /* test threads - loop infinitely without yielding CPU */
    thread_t *infinite0 = spawn_thread(
            hog_cpu, 0, PRIORITY_NORMAL, false, false);
    thread_t *infinite1 = spawn_thread(
            hog_cpu, 0, PRIORITY_NORMAL, false, false);

    /* start thread to print date/time on screen */
    thread_t* date_printer = spawn_thread(
            print_date, 0, PRIORITY_NORMAL, false, false);

    /* start thread to read keyboard input and echo it to screen */
    thread_t* echoer = spawn_thread(
            echo_input, 72, PRIORITY_NORMAL, false, false);

    thread_t* mouser = spawn_thread(
            stat_mouse, 0, PRIORITY_NORMAL, false, false);

    thread_t *test = spawn_thread(
            usermode_test, 5, PRIORITY_NORMAL, false, true);

    /* wait for some thread to finish (forever) */
    join(date_printer);

    /* playing around */
    /* kprintf("%u\n", 1 / 0); */

    /* beep(2); */
    /* delay(500); */
    /* kreboot(); */

    /* kcls(); */
    kprintf("Goodbye!");
}

static void print_date(uint32_t arg)
{
    (void)arg;
    unsigned int row, col;
    struct tm dt;
    while (true) {
        datetime(&dt);
        kget_cursor(&row, &col);
        kset_cursor(arg, 58);
        kprintf("%02u:%02u:%02u %s %02u, %04u", dt.hour, dt.min, dt.sec,
                month_name(dt.month), dt.mday, dt.year);
        kset_cursor(row, col);
        sleep(200);
    }
}

void echo_input(uint32_t arg)
{
    (void)arg;
    unsigned int i = 0;
    for (i = 0; i < arg; i++) {
        keycode_t kc = wait_for_key();
        if (kc == 'q') {
            break;
        }
        kputc(kc);
    }
    kprintf("\nYou've typed enough!\n");
}

static void hog_cpu(uint32_t arg)
{
    (void)arg;
    while (true) ;
}

static void usermode_test(uint32_t arg)
{
    uprintf("usermode_test looping %u times...\n", arg);

    unsigned int i = 0;
    for (i = 0; i < arg; i++) {
        char *dst = (char*)syscall_malloc(42);
        strcpy(dst, "hello");
        syscall_free(dst);

        *(char *)VIDEO_ADDR = '$';
        *(char *)(VIDEO_ADDR + 1) = WHITE_ON_BLACK;

        /* uprintf("Hello from usermode!\n"); */
        syscall_print("Hello from usermode!\n");

        syscall_sleep(1000);
    }
}
