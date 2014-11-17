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
#include "blkdev.h"
#include "initrd.h"

extern uintptr_t g_start, g_code, g_data, g_bss, g_end;

static void print_date(uint32_t arg);
static void echo_input(uint32_t arg);
static void hog_cpu(uint32_t arg);

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

void usermode_init(uint32_t arg)
{
    (void)arg;
    extern void start_user_mode(void);
    start_user_mode();

    char *dst = (char*)syscall_malloc(42);
    strcpy(dst, "hello");
    syscall_print("strcpy successful!");

    *(char *)VIDEO_ADDR = '$';
    *(char *)(VIDEO_ADDR + 1) = WHITE_ON_BLACK;

    extern int get_ring(void);
    /* DEBUGF("Making syscall in ring %d\n", get_ring()); */
    syscall_print("Hello from usermode!");

    /* DEBUGF("Making syscall in ring %d\n", get_ring()); */
    syscall_print("Still in usermode!");

    /* if (!interrupts_enabled()) { */
    /*     syscall_print("Interrupts disabled!"); */
    /* } else { */
    /*     syscall_print("Interrupts enabled!"); */
    /* } */

    while (1) ;
}

void kmain(struct multiboot_info *mbinfo, multiboot_uint32_t mboot_magic)
{
    /* interrupts are disabled */

    kcls();
    kprintf("Welcome to DuneOS...\n\n");
    kprintf("ESP: 0x%X\n", get_esp());

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

    kprintf("ESP: 0x%X\n", get_esp());

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

    void* stack = alloc_page();
    kprintf("Allocated new kernel stack: 0x%X\n", stack);
    set_kernel_stack((uintptr_t)stack + PAGE_SIZE);
    /* Test ISR handler with a page fault */
    /* uintptr_t* page_fault = (uintptr_t*)0xFFFFF0000; */
    /* kprintf("page fault? 0x%x\n", *page_fault); */
    /* kprintf("page fault? %u\n", *page_fault); */

    thread_t *infinite0 = start_kernel_thread(
            hog_cpu, 0, PRIORITY_NORMAL, false);
    thread_t *infinite1 = start_kernel_thread(
            hog_cpu, 0, PRIORITY_NORMAL, false);

    /* start thread to print date/time on screen */
    thread_t* date_printer = start_kernel_thread(
            print_date, 0, PRIORITY_NORMAL, false);

    /* start thread to read keyboard input and echo it to screen */
    thread_t* echoer = start_kernel_thread(
            echo_input, 0, PRIORITY_NORMAL, false);

    /* thread_t *init = start_kernel_thread( */
    /*         usermode_init, 0, PRIORITY_NORMAL, false); */

    /* wait for child threads to finish (forever) */
    /* join(init); */
    join(echoer);
    join(date_printer);
    join(infinite0);
    join(infinite1);

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
        sleep(50);
        datetime(&dt);
        kget_cursor(&row, &col);
        kset_cursor(0, 58);
        kprintf("%02u:%02u:%02u %s %02u, %04u\n", dt.hour, dt.min, dt.sec,
                month_name(dt.month), dt.mday, dt.year);
        kset_cursor(row, col);
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

static void hog_cpu(uint32_t arg)
{
    (void)arg;
    unsigned int i = 0, j = 0;
    while (true) {
        ;
    }
}
