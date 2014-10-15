#include "print.h"
#include "idt.h"
#include "x86.h"
#include "syscall.h"

static void print(const char *msg) {
    kprintf("%s\n", msg);
}


DEFN_SYSCALL1(print, 0, const char*)

static void *syscalls[] = {
    &print,
};
size_t num_syscalls = sizeof(syscalls) / sizeof(*syscalls);

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
