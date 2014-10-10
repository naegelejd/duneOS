#include "print.h"
#include "idt.h"
#include "syscall.h"

static void syscall_handler(struct regs *r)
{
    dbgprintf("Handling syscall");
    while (1);
}

void syscalls_install(void)
{
    int_install_handler(128, syscall_handler);
}
