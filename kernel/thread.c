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

static thread_t* all_threads_head;
static thread_t* all_threads_tail;

/* Queue of runnable threads */
static thread_queue_t run_queue;

/* Queue of finished threads needing disposal */
static thread_queue_t graveyard_queue;

/* Queue for reaper thread to communicate with dead threads */
static thread_queue_t reaper_wait_queue;

/* the global, currently-running thread */
static thread_t *g_current_thread;

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

}

/*
 * Remove a thread from the list of all threads
 */
static void all_threads_remove(thread_t* thread)
{

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

}

/*
 * Remove a thread from a thread queue
 */
static void dequeue_thread(thread_queue_t* queue, thread_t* thread)
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
        unsigned int priority, bool detached)
{
    static unsigned int next_free_id = 1;

    thread_t* owner = detached ? NULL : g_current_thread;

    memset(thread, 0, sizeof(thread_t));
    thread->stack_page = stack_page;
    thread->esp = (uintptr_t)stack_page + PAGE_SIZE;
    thread->priority = priority;
    thread->owner = owner;

    thread->refcount = detached ? 1 : 2;
    thread->alive = true;

    clear_thread_queue(thread->joinq);

    thread->id = next_free_id++;
}

/*
 * Create new raw thread object.
 * @returns NULL if out of memory
 */
static thread_t* create_thread(unsigned int priority, bool detached)
{
    thread_t* thread;
    void* stack_page = NULL;

    thread = alloc_page();
    if (!thread) {
        return NULL;
    }

    stack_page = alloc_page();
    if (!stack_page) {
        free_page(thread);
        return NULL;
    }

    init_thread(thread, stack_page, priority, detached);

    /* TODO: append to all thread list */
    all_threads_add(thread);

    return thread;
}

/*
 * Perform all necessary cleanup and destroy thread.
 * call with interrupts enabled.
 */
static void destroy_thread(thread_t* thread)
{
    kcli();
    free_page(thread->stack_page);
    free_page(thread);

    all_threads_remove(thread);

    ksti();
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
    ksti();
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
    while (true) {
        yield();
    }
}



void yield(void)
{
    kcli();
    //make_runnable(g_current_thread);
    //schedule();
    ksti();
}

void exit(int exit_code)
{

}

void schedule(void)
{

}

void wait(thread_queue_t* wait_queue)
{

}

void wake_up(thread_queue_t* wait_queue)
{

}

void wake_up_one(thread_queue_t* wait_queue)
{

}
