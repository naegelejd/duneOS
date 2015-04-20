Thread stack after initialization:

- arg to thread function
- address of shutdown thread (return address)
- address of start function
- EFLAGS (zero)
- CS (kernel code segment selector)
- address of launch thread (return address used by `iret`)
- fake interrupt error code (zero)
- fake interrupt number     (zero)
- eax   (zero)
- ebx   (zero)
- ecx   (zero)
- edx   (zero)
- esi   (zero)
- edi   (zero)
- ebp   (zero)
- DS    (kernel data segment selector)
- CS    (kernel code segment selector)
- FS    (zero)
- ES    (zero)

thread switch procedure:

- push eax on stack
- move return address down stack 8 bytes (using eax)
- adjust stack
- push EFLAGS
- restore eax
- push CS (kernel code segment selector)
-



main:
- initialize all x86 stuff
- call `scheduler_init`
- call `start_kernel_thread` with `echo_input` function
- call `start_kernel_thread` with `print_date` function
- call `join` on the "echo input" thread
- call `join` on the "print date" thread

scheduler_init:
- calls `init_thread` on main thread (allocated in BSS), passing BSS-allocated
  page for stack

init_thread:
- zero thread struct
- set stack base (1 page) and `esp`
- set priority, owner, refcount, alive
- clear join queue
- set ID
- return to `scheduler_init`

scheduler_init:
- add main thread to list of all threads
- call `start_kernel_thread` with `idle` function and idle priority
- call `start_kernel_thread` with `reaper` function and idle priority

start_kernel_thread:
- call `create_thread`

    create_thread:
    - allocate a page for the thread struct
    - allocate a page for the thread's stack
    - call `init_thread`
    - add thread to list of all threads
    - return to `start_kernel_thread`
- call `setup_kernel_thread`

    setup_kernel_thread:
    -  push start function arg (uint32_t) on thread's stack
    -  push pointer to `shutdown_thread` on thread's stack
    -  push pointer to start function on thread's stack
    -  push everything needed to pretend it was suspended by an interrupt in user mode:
        - EFLAGS (0)
        - code segment selector (0x08)
        - return address (`launch_thread` function pointer)
        - fake error code, fake interrupt number (0, 0)
        - general registers (0s for eax, ebx, ecx, edx, esi, edi, ebp)
        - segment registers (data, code, fs, gs = 0x10, 0x08, 0, 0)

    - return to `start_kernel_thread`
- add thread to "runnable" queue

join:
- adds current thread to the join queeu

idle:
- calls `yield()` in infinite loop

yield:
- adds current thread to `runnable` queue
- calls `schedule`

reaper:
- cleans up dead threads and `yield()`s

