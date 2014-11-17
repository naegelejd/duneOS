#include "syscall.h"

#include "print.h"
#include "idt.h"
#include "x86.h"
#include "mem.h"

static void print(const char *msg) {
    kprintf("%s\n", msg);
}

#include "timer.h"
static void sleep(unsigned int msec) {
    unsigned int ticks = msec / 10;
    if (ticks < 1) { ticks = 1; }
    delay(ticks);
}

DEFN_SYSCALL1(print, 0, const char*)
DEFN_SYSCALL1(sleep, 1, unsigned int)
DEFN_SYSCALL1(malloc, 2, size_t)

static void *syscalls[] = {
    &print,
    &sleep,
    &malloc,
};
size_t num_syscalls = sizeof(syscalls) / sizeof(*syscalls);

#include "thread.h"
static void syscall_handler(struct regs *regs)
{
    uint32_t *esp = get_current_thread()->esp;
    uint32_t *kesp = get_current_thread()->kernel_esp;
    DEBUGF("ESP: %X\n%X\n%X\n%X\n%X\n%X\n", esp, *esp, *(esp+1), *(esp+2), *(esp+3), *(esp+4));

    if (regs->eax >= num_syscalls) {
        kprintf("%s\n", "Invalid syscall number");
        return;
    }

    extern int get_ring(void);
    DEBUGF("Handling syscall in ring %d\n", get_ring());

    void *location = syscalls[regs->eax];

    int ret;
    asm volatile (" \
      push %1; \
      push %2; \
      push %3; \
      push %4; \
      push %5; \
      call *%6; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
    " : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
    regs->eax = ret;
}

void syscalls_install(void)
{
    int_install_handler(128, syscall_handler);
}
