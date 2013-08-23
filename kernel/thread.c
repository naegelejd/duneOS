/* This file was adapted from GeekOS "kthread.h":

    Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
    Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
    ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
    TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
    SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
    FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
    THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "seg.h"
#include "mem.h"
#include "int.h"
#include "string.h"
#include "thread.h"

/* List of all threads in the system */
static thread_queue_t all_threads;

/* Queue of runnable threads */
static thread_queue_t run_queue;

/* Queue of finished threads needing disposal */
static thread_queue_t graveyard_queue;

/* Queue for reaper thread to communicate with dead threads */
static thread_queue_t reaper_wait_queue;

/* global, currently-running thread */
thread_t *g_current_thread;

/* When set, the scheduler needs to choose a new runnable thread */
bool g_need_reschedule;

/* When set, interrupts will not cause a new thread to be scheduled */
volatile bool g_preemption_disabled;


/*
 * Counter and array of destructors for keys that access
 * thread-local data (Based on POSIX threads' thread-specific data)
 */
static unsigned int tlocal_key_counter;
static tlocal_destructor_t tlocal_destructors[MAX_TLOCAL_KEYS];


/*
 * Add a thread to the list of all threads
 */
static void all_threads_add(thread_t* thread)
{
    if (all_threads.head == NULL) {
        all_threads.head = thread;
    } else if (all_threads.tail != NULL) {
        all_threads.tail->list_next = thread;
    }
    thread->list_next = NULL;
    all_threads.tail = thread;
}

/*
 * Remove a thread from the list of all threads
 */
static void all_threads_remove(thread_t* thread)
{
    thread_t** t = &all_threads.head;
    while (*t) {
        if (thread == *t) {
            *t = (*t)->list_next;
        }
        t = &(*t)->list_next;
    }
}


/*
 * 'Empty' a thread queue.
 */
static void clear_thread_queue(thread_queue_t* queue)
{
    queue->head = NULL;
    queue->tail = NULL;
}

/*
 * Add a thread to a thread queue.
 */
static void enqueue_thread(thread_queue_t* queue, thread_t* thread)
{
    if (queue->head == NULL) {
        queue->head = thread;
    } else if (queue->tail != NULL) {
        queue->tail->queue_next = thread;
    }

    thread->queue_next = NULL;
    queue->tail = thread;
}

/*
 * Remove a thread from a thread queue
 * LOL @ no temporary 'prev' pointer, thanks Linus...
 */
static void dequeue_thread(thread_queue_t* queue, thread_t* thread)
{
    thread_t** t = &queue->head;
    while (*t) {
        if (thread == *t) {
            *t = (*t)->queue_next;
        }
        t = &(*t)->queue_next;
    }
}

/*
 * Clean up thread-local data.
 * Assumes interrupts disabled
 */
static void tlocal_exit(thread_t* thread)
{

}

/*
 * Push a 32-bit value onto the thread's stack.
 * Used for initializing threads.
 */
static inline void push_dword(thread_t* thread, uint32_t value)
{
    thread->esp -= 4;
    *(uint32_t*)thread->esp = value;
}

/*
 * Initialize members of a kernel thread
 */
static void init_thread(thread_t* thread, void* stack_page,
        priority_t priority, bool detached)
{
    static unsigned int next_free_id = 0;

    thread_t* owner = detached ? NULL : g_current_thread;

    memset(thread, 0, sizeof(thread_t));
    thread->stack_page = stack_page;
    /* TODO: don't assume thread stack is PAGE_SIZE bytes */
    thread->esp = (uintptr_t)stack_page + PAGE_SIZE;
    thread->priority = priority;
    thread->owner = owner;

    thread->refcount = detached ? 1 : 2;
    thread->alive = true;

    clear_thread_queue(&thread->join_queue);

    thread->id = next_free_id++;
    dbgprintf("New thread pid: %d\n", thread->id);
}

/*
 * Create new raw thread object.
 * @returns NULL if out of memory
 */
static thread_t* create_thread(unsigned int priority, bool detached)
{
    thread_t* thread = NULL;
    void* stack_page = NULL;

    thread = alloc_page();
    dbgprintf("Allocated page @ 0x%x for thread context\n", thread);
    if (!thread) {
        kprintf("Failed to allocate page for thread\n");
        return NULL;
    }

    stack_page = alloc_page();
    dbgprintf("Allocated page @ 0x%x for thread stack\n", stack_page);
    if (!stack_page) {
        kprintf("Failed to allocate page for thread stack\n");
        free_page(thread);
        return NULL;
    }

    init_thread(thread, stack_page, priority, detached);

    all_threads_add(thread);

    return thread;
}

/*
 * Perform all necessary cleanup and destroy thread.
 * call with interrupts enabled.
 */
static void destroy_thread(thread_t* thread)
{
    cli();
    free_page(thread->stack_page);
    free_page(thread);

    all_threads_remove(thread);

    sti();
}

/*
 * pass thread to reaper for destruction
 * must be called with interrupts disabled
 */
static void reap_thread(thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    enqueue_thread(&graveyard_queue, thread);
    wake_up(&reaper_wait_queue);
}

/*
 * called when a reference to a thread is broken
 */
static void detach_thread(thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    KASSERT(thread->refcount > 0);

    --thread->refcount;
    if (thread->refcount == 0) {
        reap_thread(thread);
    }
}

/*
 * Perform any necessary initialization before a thread start function
 * is executed. It currently only enables interrupts
 */
static void launch_thread(void)
{
    sti();
}

/*
 * Push initial values for saved general-purpose registers
 */
static void push_general_registers(thread_t* thread)
{
    push_dword(thread, 0);    /* eax */
    push_dword(thread, 0);    /* ebx */
    push_dword(thread, 0);    /* ecx */
    push_dword(thread, 0);    /* edx */
    push_dword(thread, 0);    /* esi */
    push_dword(thread, 0);    /* edi */
    push_dword(thread, 0);    /* ebp */
}

/*
 * Shutdown a kernel thread if it exits by falling
 * off the end of its start function
 */
static void shutdown_thread(void)
{
    exit(0);
}

/*
 * Set up the initial context for a kernel-mode only thread
 */
static void setup_kernel_thread(thread_t* thread,
        thread_start_func_t start_func, uint32_t arg)
{
    /* push the arg to the thread start function */
    push_dword(thread, arg);

    /* push the address of the shutdown_thread function as the
     * return address. this forces the thread to exit */
    push_dword(thread, (uintptr_t)&shutdown_thread);

    /* push the address of the start function */
    push_dword(thread, (uintptr_t)start_func);

    /* to make the thread schedulable, we need to make it look like
     * it was suspended by an interrupt in user mode.
     * so push all the interrupt stack values onto the stack */
    push_dword(thread, 0UL);  /* EFLAGS */

    /* push address of the launch_thread function as the "return address",
     * so when we call "iret", it jumps to launch_thread */
    push_dword(thread, CODE_SEG_SELECTOR);
    push_dword(thread, (uintptr_t)&launch_thread);

    /* push fake error code and INT number */
    push_dword(thread, 0);
    push_dword(thread, 0);

    push_general_registers(thread);

    /* push values for saved segment registers.
     * only the DS and ES registers contain valid selectors
     * (FS and GS not used by GCC) */
    push_dword(thread, DATA_SEG_SELECTOR);
    push_dword(thread, CODE_SEG_SELECTOR);
    push_dword(thread, 0);
    push_dword(thread, 0);
}

static void idle(uint32_t arg)
{
    (void)arg; /* prevent compiler warnings */
    dbgprintf("Idle thread idling\n");
    while (true) {
        yield();
    }
}

static void reaper(uint32_t arg)
{
    thread_t* thread;
    (void)arg; /* prevent compiler warnings */
    cli();

    while (true) {
        /* check if any threads need disposal */
        if ((thread = graveyard_queue.head) == NULL) {
            /* graveyard empty... wait for thread to die */
            wait(&reaper_wait_queue);
        } else {
            /* empty the graveyard queue */
            clear_thread_queue(&graveyard_queue);

            /* re-enable interrupts. all threads needing disposal
             * have been disposed. */
            sti();
            yield();    /* allow other threads to run? */

            while (thread != 0) {
                thread_t* next = thread->queue_next;
                dbgprintf("Reaper destroying thread: 0x%x\n", thread);
                destroy_thread(thread);
                thread = next;
            }

            /* disable interrupts again for another iteration of diposal */
            cli();
        }
    }
}

void yield(void)
{
    cli();
    make_runnable(g_current_thread);
    schedule();
    sti();
}

/*
 * Exit current thread and initiate a context switch
 */
void exit(int exit_code)
{
    thread_t* current = g_current_thread;

    if (interrupts_enabled()) {
        cli();
    }

    current->exit_code = exit_code;
    current->alive = false;

    /* clean up thread-local data */
    tlocal_exit(current);

    /* notify thread's possible owner */
    wake_up(&current->join_queue);

    /* remove thread's implicit reference to itself */
    detach_thread(current);

    schedule();

    KASSERT(false);
}

thread_t* find_best(thread_queue_t* queue)
{
    return queue->head;
}

thread_t* get_next_runnable(void)
{
    thread_t* best = find_best(&run_queue);
    KASSERT(best != NULL);
    dequeue_thread(&run_queue, best);

    return best;
}

/*
 * Schedule a runnable thread.
 * Called with interrupts disabled.
 * g_current_thread should already be place on another
 * queue (or left on run queue)
 */
extern void switch_to_thread(thread_t*);
void schedule(void)
{
    KASSERT(!interrupts_enabled());
    KASSERT(!g_preemption_disabled);

    thread_t* runnable = get_next_runnable();

    dbgprintf("Switching to thread 0x%x\n", runnable);
    switch_to_thread(runnable);
}

/*
 * Wait for a thread to die.
 * Interrupts must be enabled.
 *
 * @returns thread exit code
 */
int join(thread_t* thread)
{
    KASSERT(interrupts_enabled());

    /* only the owner can join on a thread */
    KASSERT(thread->owner = g_current_thread);

    cli();

    while (thread->alive) {
        wait(&thread->join_queue);
    }

    int exitcode = thread->exit_code;

    /* release reference to thread */
    detach_thread(thread);

    sti();

    return exitcode;
}

/*
 * Place current thread on a wait queue
 * Called with interrupts disabled.
 */
void wait(thread_queue_t* wait_queue)
{
    KASSERT(!interrupts_enabled());

    thread_t* current = g_current_thread;

    enqueue_thread(wait_queue, current);

    schedule();
}

/*
 * Wake up all threads waiting on a wait queue.
 * Called with interrupts disabled.
 */
void wake_up(thread_queue_t* wait_queue)
{
    thread_t* thread = wait_queue->head;
    thread_t* next = NULL;

    while (thread) {
        next = thread->queue_next;
        make_runnable(thread);
        thread = next;
    }

    clear_thread_queue(wait_queue);
}

void wake_up_one(thread_queue_t* wait_queue)
{
    KASSERT(!interrupts_enabled());
    thread_t* best = find_best(wait_queue);
    if (best != NULL) {
        dequeue_thread(wait_queue, best);
        make_runnable(best);
    }
}

thread_t* start_kernel_thread(thread_start_func_t start_function, uint32_t arg,
        priority_t priority, bool detached)
{
    thread_t* thread = create_thread(priority, detached);

    KASSERT(thread != NULL);    /* was thread created? */
    if (thread) {
        setup_kernel_thread(thread, start_function, arg);

        make_runnable_atomic(thread);
    }
    return thread;
}

/*
 * Add thread to run queue so it will be scheduled
 */
void make_runnable(thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    enqueue_thread(&run_queue, thread);
}

/*
 * Assumes interrupts are enabled before
 * atomically making thread runnable
 */
void make_runnable_atomic(thread_t* thread)
{
    cli();
    make_runnable(thread);
    sti();
}

thread_t* get_current_thread(void)
{
    return g_current_thread;
}

void scheduler_init(void)
{
    extern char main_thread_addr, kernel_stack_bottom;
    thread_t* main_thread = (thread_t*)&main_thread_addr;

    init_thread(main_thread, (void*)&kernel_stack_bottom, PRIORITY_NORMAL, true);
    g_current_thread = main_thread;
    all_threads_add(g_current_thread);

    start_kernel_thread(idle, 0, PRIORITY_IDLE, true);

    start_kernel_thread(reaper, 0, PRIORITY_NORMAL, true);
}

void dump_all_threads_list(void)
{
    thread_t* thread;
    int count = 0;
    bool iflag = beg_int_atomic();

    thread = all_threads.head;

    kprintf("[");
    while (thread != NULL) {
        ++count;
        kprintf("<%x %x>", (uintptr_t)thread, (uintptr_t)thread->list_next);
        //KASSERT(thread != thread->list_next);
        thread = thread->list_next;
    }
    kprintf("]\n");
    kprintf("%d threads are running\n", count);

    end_int_atomic(iflag);
}
