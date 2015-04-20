# Multitasking

## Task structure

- stack pointer
- base pointer?         (save on stack?)
- instruction pointer?  (save on stack?)
- page directory
- kernel stack (for userspace tasks)
- timer tick counter
- priority
- ID
- reference count
- pointer to parent task
- pointer to next task in list of all tasks
- pointer to next task in current queue
- join queue for waiting tasks
- 'alive' flag
- sleep duration
- exit code

### Kernel

- page directory is just kernel page directory
- kernel stack not necessary

### Userspace

- each task needs a new page directory with kernel mapped in
- each task needs its own kernel stack. the `esp0` field of the
  global TSS structure must be set for each task, as this stack
  is needed by the kernel to handle interrupts (including syscalls)

## Loading tasks

### Kernel
Kernel tasks can be created using any entry point.

### Userspace
Userspace tasks need a new page directory:
- stack starting at 0xC0000000-4 and growing downward
- code/data starting at 0x00000000?
- kernel mapped at 0xC0000000 (copied from kernel page directory)
TODO: Userspace tasks need an entry point

Pages must be allocated by the kernel for the following:
- task code/data
- task stack
- kernel stack

## Task switching

Preemption

1. PIT triggers an interrupt every X milliseconds
1. Kernel timer counts interrupts as ticks
1. After Y ticks, the current task is placed on the runnable queue
   and the scheduler is called to start the next task

Cooperative

1. Tasks can `yield`, causing the kernel to schedule the next task
1. Tasks can `sleep`, causing the kernel to wait a period of time
   before placing the task back on the runnable queue.

Switch procedure

1. Save all registers on stack
1. Point `g_current_task` to new task struct
1. Set `esp` to current task's stack pointer
1. Restore registers from stack

## Interrupts

### Kernel

If an interrupt occurs while a kernel task is running, nothing
special needs to happen.

### Userspace

If an interrupt occurs while a userspace task is running, the
`esp0` field of the TSS should already have been set to the kernel
stack of the task. This will allow the kernel to handle the interrupt
and return execution to the task.

## Syscalls

Userspace tasks can "request" to execute kernel code by triggering
interrupt 0x80 (like Linux), the only ring-3 interrupt. The handler
for this interrupt will call the corresponding kernel code.

## Task cleanup

Userspace tasks are allocated pages for code/data, and both user/kernel
stacks. These must all be freed to the system when a task completes.
