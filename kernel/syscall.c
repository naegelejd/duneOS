#include "syscall.h"

#include "print.h"
#include "idt.h"
#include "x86.h"
#include "mem.h"
#include "thread.h"

static void print(const char *msg) {
    kprintf("%s", msg);
}

DEFN_SYSCALL1(print, 0, const char*)
DEFN_SYSCALL1(sleep, 1, unsigned int)
DEFN_SYSCALL1(malloc, 2, size_t)
DEFN_SYSCALL1(free, 3, void*)
DEFN_SYSCALL1(exit, 4, int)

static void *syscalls[] = {
    &print,
    &sleep,
    &malloc,
    &free,
    &exit
};
size_t num_syscalls = sizeof(syscalls) / sizeof(*syscalls);

#include "thread.h"
static void syscall_handler(struct regs *regs)
{
    if (regs->eax >= num_syscalls) {
        kprintf("%s\n", "Invalid syscall number");
        return;
    }

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
