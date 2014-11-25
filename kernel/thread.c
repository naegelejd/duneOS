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

#include "x86.h"
#include "gdt.h"
#include "mem.h"
#include "int.h"
#include "timer.h"
#include "string.h"
#include "thread.h"

/* List of all threads in the system */
static thread_t* all_threads_head;

/* Queue of runnable threads */
static thread_queue_t run_queue;

/* Queue of sleeping threads */
static thread_queue_t sleep_queue;

/* Queue of finished threads needing disposal */
static thread_queue_t graveyard_queue;

/* Queue for reaper thread to communicate with dead threads */
static thread_queue_t reaper_wait_queue;

/* global, currently-running thread */
thread_t *g_current_thread = NULL;

/* When set, interrupts will not cause a new thread to be scheduled */
static volatile bool g_preemption_disabled;


/* Counter for keys that access thread-local data
 * (Based on POSIX threads' thread-specific data) */
static unsigned int tlocal_key_counter;
/* Array of destructors for correspondingly keyed thread-local data */
static tlocal_destructor_t tlocal_destructors[MAX_TLOCAL_KEYS];

typedef void (thread_launch_func_t)(void);

/*
 * Add a thread to the list of all threads
 */
static void all_threads_add(thread_t* thread)
{
    KASSERT(thread);

    thread->list_next = NULL;
    thread_t **t = &all_threads_head;
    while (*t != NULL) {
        KASSERT(*t != thread);
        t = &(*t)->list_next;
    }
    *t = thread;
}

/*
 * Remove a thread from the list of all threads
 */
static void all_threads_remove(thread_t* thread)
{
    KASSERT(thread);

    thread_t** t = &all_threads_head;
    while (*t != NULL) {
        if (thread == *t) {
            *t = (*t)->list_next;
            break;
        }
        t = &(*t)->list_next;
    }
    thread->list_next = NULL;
}


/*
 * 'Empty' a thread queue.
 */
void thread_queue_clear(thread_queue_t* queue)
{
    KASSERT(queue);
    queue->head = NULL;
    queue->tail = NULL;
}

bool thread_queue_empty(thread_queue_t* queue)
{
    KASSERT(queue);
    if (queue->head == NULL && queue->tail == NULL) {
        return true;
    }
    return false;
}

static bool contains_thread(thread_queue_t* queue, thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    KASSERT(queue);
    KASSERT(thread);

    thread_t *cur = queue->head;
    while (cur) {
        if (cur == thread) {
            return true;
        }
        cur = cur->queue_next;
    }

    KASSERT(thread != queue->tail); /* paranoia */

    return false;
}

/*
 * Add a thread to a thread queue.
 */
static void enqueue_thread(thread_queue_t* queue, thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    KASSERT(queue);
    KASSERT(thread);

    /* ensure thread points to no next thread */
    thread->queue_next = NULL;

    /* overkill - make sure thread is not already in queue */
    KASSERT(!contains_thread(queue, thread));

    if (NULL == queue->head) {
        KASSERT(NULL == queue->tail);   /* paranoia */
        queue->head = thread;
        queue->tail = thread;
    } else if (queue->head == queue->tail) {
        KASSERT(NULL == queue->head->queue_next);   /* paranoia */
        queue->tail->queue_next = thread;
        queue->tail = thread;
    } else {
        queue->tail->queue_next = thread;
        queue->tail = thread;
    }
}

/*
 * Very carefully remove all instances of a thread from a queue
 */
static void dequeue_thread(thread_queue_t* queue, thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    KASSERT(queue);
    KASSERT(thread);

    thread_t* cur = queue->head;
    thread_t* prev = NULL;
    while (cur) {
        if (thread == cur) {
            if (cur == queue->head) {
                /* remove head */
                queue->head = cur->queue_next;
            } else {
                /* remove thread in the middle */
                prev->queue_next = cur->queue_next;
            }
        } else {
            /* increment 'prev' only if cur was NOT removed */
            prev = cur;
        }
        cur = cur->queue_next;
    }

    queue->tail = prev;

    /* ensure thread is no longer pointing to its former 'next' thread */
    thread->queue_next = NULL;
}

/*
 * Clean up thread-local data.
 * Calls destructors *repeatedly* until all thread-local data is NULL.
 * Assumes interrupts disabled.
 */
static void tlocal_exit(thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    KASSERT(thread);

    bool repeat = false;
    do {
        repeat = false;     /* assume we don't need to repeat */
        int idx;
        for (idx = 0; idx < MAX_TLOCAL_KEYS; idx++) {
            void* data = (void*)thread->tlocal_data[idx];
            tlocal_destructor_t destructor = tlocal_destructors[idx];

            if (data != NULL && destructor != NULL) {
                thread->tlocal_data[idx] = NULL;
                repeat = true;  /* need to call all destructors again */
                sti();
                tlocal_destructors[idx](data);
                cli();
            }
        }
    } while (repeat);
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
        void* user_stack_page, priority_t priority, bool detached)
{
    static unsigned int next_free_id = 0;

    memset(thread, 0, sizeof(thread_t));

    thread->id = next_free_id++;

    thread->stack_base = stack_page;
    /* TODO: don't assume thread stack is PAGE_SIZE bytes */
    thread->esp = (uintptr_t)stack_page + PAGE_SIZE;
    thread->stack_top = thread->esp;

    thread->user_stack_base = user_stack_page;
    thread->user_esp = (user_stack_page != NULL) ?
            (uintptr_t)user_stack_page + PAGE_SIZE : 0;

    thread->priority = priority;
    thread->owner = detached ? NULL : g_current_thread;

    thread->refcount = detached ? 1 : 2;
    thread->alive = true;

    thread_queue_clear(&thread->join_queue);

    DEBUGF("new thread @ 0x%X, id: %d, esp: %X\n",
            thread, thread->id, thread->esp, thread->user_esp);
}

/*
 * Create new raw thread object.
 * @returns NULL if out of memory
 */
static thread_t* create_thread(unsigned int priority, bool detached, bool usermode)
{
    thread_t *thread = alloc_page();
    DEBUGF("Allocated thread 0x%X\n", thread);
    if (!thread) {
        kprintf("Failed to allocate page for thread\n");
        return NULL;
    }

    void *stack_page = alloc_page();
    if (!stack_page) {
        kprintf("Failed to allocate page for thread stack\n");
        free_page(thread);
        return NULL;
    }

    void *user_stack_page = NULL;
    if (usermode) {
        user_stack_page = alloc_page();
        if (!user_stack_page) {
            kprintf("Failed to allocate page for thread user stack\n");
            free_page(stack_page);
            free_page(thread);
            return NULL;
        }
    }

    init_thread(thread, stack_page, user_stack_page, priority, detached);

    all_threads_add(thread);

    return thread;
}

/*
 * Perform all necessary cleanup and destroy thread.
 * call with interrupts enabled.
 */
static void destroy_thread(thread_t* thread)
{
    KASSERT(thread);
    cli();

    free_page(thread->stack_base);
    if (thread->user_stack_base) {
        free_page(thread->user_stack_base);
    }
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
    KASSERT(thread);
    DEBUGF("reaping thread %d\n", thread->id);
    enqueue_thread(&graveyard_queue, thread);
    wake_all(&reaper_wait_queue);
}

/*
 * called when a reference to a thread is broken
 */
static void detach_thread(thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    KASSERT(thread);
    KASSERT(thread->refcount > 0);

    DEBUGF("detaching thread %d\n", thread->id);

    --thread->refcount;
    if (thread->refcount == 0) {
        reap_thread(thread);
    }
}

/*
 * Perform any necessary initialization before a thread start function
 * is executed. It currently only enables interrupts
 */
static void launch_kernel_thread(void)
{
    /* DEBUGF("Launching thread %d\n", g_current_thread->id); */
    /* DEBUGF("%s\n", interrupts_enabled() ? "interrupts enabled\n" : "interrupts disabled\n"); */
    sti();
}

static void launch_user_thread(void)
{
    /* Now, 'start' user mode */
    extern void start_user_mode();
    start_user_mode();
}

/*
 * Shutdown a kernel thread if it exits by falling
 * off the end of its start function
 */
static void shutdown_kernel_thread(void)
{
    /* DEBUGF("Shutting down thread %d\n", g_current_thread->id); */
    exit(0);
}

static void shutdown_user_thread(void)
{
    syscall_exit(0);
}

static void setup_thread_stack(thread_t* thread,
        thread_start_func_t start_func, uint32_t arg, bool usermode)
{
    uint32_t *esp = (uint32_t*)thread->esp;

    if (usermode) {
        /* Set up CPL=3 stack */
        uint32_t *uesp = thread->user_esp;
        KASSERT(uesp != NULL);

        /* push the arg to the thread start function */
        *--uesp = arg;

        /* push the address of the shutdown_thread function as the
        * return address. this forces the thread to exit */
        *--uesp = shutdown_user_thread;

        thread->user_esp = uesp;

        /* Set up CPL=0 stack */
        /* DEBUGF("Address of start_func: %X\n", start_func); */
        *--esp = start_func;

        extern void start_user_mode(void);
        *--esp = start_user_mode;
    } else {
        /* push the arg to the thread start function */
        *--esp = arg;

        /* push the address of the shutdown_thread function as the
        * return address. this forces the thread to exit */
        *--esp = shutdown_kernel_thread;

        *--esp = start_func;

        *--esp = launch_kernel_thread;
    }

    /* push general registers */
    *--esp = 0;     /* eax */
    *--esp = 0;     /* ecx */
    *--esp = 0;     /* edx */
    *--esp = 0;     /* ebx */
    *--esp = 0;     /* ebp */
    *--esp = 0;     /* esi */
    *--esp = 0;     /* edi */

    /* update thread's ESP */
    thread->esp = esp;
}

static void idle(uint32_t arg)
{
    (void)arg; /* prevent compiler warnings */
    DEBUG("Idle thread idling\n");
    while (true) {
        yield();
    }
}

static void reaper(uint32_t arg)
{
    thread_t* thread;
    (void)arg; /* prevent compiler warnings */
    DEBUG("Reaper thread reaping\n");
    cli();

    while (true) {
        /* check if any threads need disposal */
        if ((thread = graveyard_queue.head) == NULL) {
            /* graveyard empty... wait for thread to die */
            wait(&reaper_wait_queue);
        } else {
            /* empty the graveyard queue */
            thread_queue_clear(&graveyard_queue);

            /* re-enable interrupts. all threads needing disposal
             * have been disposed. */
            sti();
            yield();    /* allow other threads to run? */

            while (thread != 0) {
                thread_t* next = thread->queue_next;
                DEBUGF("Reaper destroying thread: 0x%x\n", thread);
                destroy_thread(thread);
                thread = next;
            }

            /* disable interrupts again for another iteration of diposal */
            cli();
        }
    }
}

/*
 * Wake up any threads that are finished sleeping
 */
void wake_sleepers(void)
{
    thread_t* next = NULL;
    thread_t* thread = sleep_queue.head;

    while (thread) {
        /* DEBUGF("attempting to wake thread %d\n", thread->id); */
        next = thread->queue_next;
        if (get_ticks() >= thread->sleep_until) {
            /* DEBUGF("waking thread %d (%u >= %u)\n", */
                    /* thread->id, get_ticks(), thread->sleep_until); */
            thread->sleep_until = 0;
            /* time to wake up this sleeping thread...
             * so remove it from sleep queue and make it runnable */
            dequeue_thread(&sleep_queue, thread); /* FIXME - dumb inefficient */
            make_runnable(thread);
        } else {
            /* DEBUGF("thread %d still sleeping (%u < %u)\n", */
                    /* thread->id, get_ticks(), thread->sleep_until); */
        }
        thread = next;
    }
}

/*
 * Find best candidate thread in a thread queue.
 *
 * This currently returns the head of the thread queue,
 * so the scheduling algorithm is essentially FIFO
 */
static thread_t* find_best(thread_queue_t* queue)
{
    KASSERT(queue);
    return queue->head;
}

thread_t* get_next_runnable(void)
{
    thread_t* best = find_best(&run_queue);
    KASSERT(best);
    dequeue_thread(&run_queue, best);

    return best;
}


/*
 * Determine a new key and set the destructor for thread-local data.
 *
 * @returns true on success, false on failure (no more keys)
 */
bool tlocal_create(tlocal_key_t* key, tlocal_destructor_t destructor)
{
    KASSERT(key);

    bool iflag = beg_int_atomic();

    if (tlocal_key_counter >= MAX_TLOCAL_KEYS) {
        return false;
    }

    tlocal_destructors[tlocal_key_counter] = destructor;
    *key = tlocal_key_counter++;

    end_int_atomic(iflag);

    return true;
}

void tlocal_set(tlocal_key_t key, const void* data)
{
    KASSERT(key < tlocal_key_counter);
    g_current_thread->tlocal_data[key] = data;
}

void* tlocal_get(tlocal_key_t key)
{
    KASSERT(key < tlocal_key_counter);
    return (void*)g_current_thread->tlocal_data[key];
}


void yield(void)
{
    KASSERT(g_current_thread);
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
    KASSERT(current);

    if (interrupts_enabled()) {
        cli();
    }

    current->exit_code = exit_code;
    current->alive = false;

    /* clean up thread-local data */
    tlocal_exit(current);

    /* notify thread's possible owner */
    wake_all(&current->join_queue);

    /* remove thread's implicit reference to itself */
    detach_thread(current);

    schedule();

    /* This thread will never be scheduled again, so it should
     * never get to this point */
    KASSERT(false);
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

    KASSERT(thread);
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
 * Places a thread on the sleep queue.
 * The thread will not become runnable until `ticks`
 * number of timer ticks have passed from the time
 * `sleep` is called.
 */
void sleep(unsigned int milliseconds)
{
    KASSERT(g_current_thread);

    unsigned int ticks = milliseconds * TICKS_PER_SEC / 1000;
    if (ticks < 1) { ticks = 1; }

    bool iflag = beg_int_atomic();
    g_current_thread->sleep_until = get_ticks() + ticks;
    KASSERT(!interrupts_enabled());
    enqueue_thread(&sleep_queue, g_current_thread);
    /* DEBUGF("thread %d sleeping until %u\n", g_current_thread->id, */
            /* g_current_thread->sleep_until); */
    schedule();
    end_int_atomic(iflag);
}

/*
 * Place current thread on a wait queue
 * Called with interrupts disabled.
 */
void wait(thread_queue_t* wait_queue)
{
    KASSERT(!interrupts_enabled());
    KASSERT(wait_queue);
    KASSERT(g_current_thread);

    enqueue_thread(wait_queue, g_current_thread);

    schedule();
}

/*
 * Wake up all threads waiting on a wait queue.
 * Called with interrupts disabled.
 */
void wake_all(thread_queue_t* wait_queue)
{
    thread_t* thread = wait_queue->head;
    thread_t* next = NULL;

    while (thread) {
        next = thread->queue_next;
        make_runnable(thread);
        thread = next;
    }

    thread_queue_clear(wait_queue);
}

/*
 * Wake up (one) thread that is the best candidate for running
 */
void wake_one(thread_queue_t* wait_queue)
{
    KASSERT(!interrupts_enabled());
    thread_t* best = find_best(wait_queue);
    if (best != NULL) {
        dequeue_thread(wait_queue, best);
        make_runnable(best);
    }
}

/*
 * Add thread to run queue so it will be scheduled
 */
void make_runnable(thread_t* thread)
{
    KASSERT(!interrupts_enabled());
    KASSERT(thread);
    enqueue_thread(&run_queue, thread);
}

/*
 * Assumes interrupts are enabled before
 * atomically making thread runnable
 */
void make_runnable_atomic(thread_t* thread)
{
    KASSERT(thread);

    cli();
    make_runnable(thread);
    sti();
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

    wake_sleepers();

    thread_t* runnable = get_next_runnable();

    KASSERT(runnable);
    KASSERT(g_current_thread);

    /* DEBUGF("switching from thread %d to thread %d\n", */
            /* g_current_thread->id, runnable->id); */
    switch_to_thread(runnable);
}

/*
 * Start a kernel thread with a function to execute, an unsigned
 * integer argument to that function, its priority, and whether
 * it should be detached from the current running thread.
 */
thread_t* spawn_thread(thread_start_func_t start_func, uint32_t arg,
        priority_t priority, bool detached, bool usermode)
{
    KASSERT(start_func);

    thread_t* thread = create_thread(priority, detached, usermode);
    KASSERT(thread);    /* was thread created? */

    setup_thread_stack(thread, start_func, arg, usermode);

    make_runnable_atomic(thread);

    return thread;
}


/*
 * Initialize the scheduler.
 *
 * Initializes the main kernel thread, the Idle thread,
 * and a Reaper thread for cleaning up dead threads.
 */
void scheduler_init(void)
{
    extern uintptr_t main_thread_addr, kernel_stack_bottom;
    thread_t* main_thread = (thread_t*)&main_thread_addr;
    KASSERT(main_thread);

    init_thread(main_thread, (void*)&kernel_stack_bottom,
            NULL, PRIORITY_NORMAL, true);
    g_current_thread = main_thread;
    all_threads_add(g_current_thread);

    spawn_thread(idle, 0, PRIORITY_IDLE, true, false);

    spawn_thread(reaper, 0, PRIORITY_NORMAL, true, false);
}


void dump_thread_info(thread_t* th)
{
    KASSERT(th);
    DEBUGF("esp: 0x%X\n", th->esp);
    DEBUGF("num_ticks: %u\n", th->num_ticks);
    DEBUGF("user esp: 0x%X\n", th->user_esp);
    DEBUGF("sleep_until: %u\n", th->sleep_until);
    DEBUGF("queue_next: 0x%0X\n", th->queue_next);
    DEBUGF("list_next: 0x%0X\n", th->list_next);
}

/*
 * Dumps debugging data for each thread in the list of all threads
 */
void dump_all_threads_list(void)
{
    thread_t* thread;
    int count = 0;
    bool iflag = beg_int_atomic();

    thread = all_threads_head;

    kprintf("[");
    while (thread != NULL) {
        ++count;
        kprintf("<%x %x>", (uintptr_t)thread, (uintptr_t)thread->list_next);
        thread = thread->list_next;
    }
    kprintf("]\n");
    kprintf("%d threads are running\n", count);

    end_int_atomic(iflag);
}

/*
 * Returns pointer to currently running thread
 */
thread_t* get_current_thread(void)
{
    return g_current_thread;
}

static void mutex_wait(mutex_t *mutex)
{
    KASSERT(mutex);
    KASSERT(mutex->locked);
    KASSERT(g_preemption_disabled);

    cli();
    /* re-nable preemption since we're about to wait */
    g_preemption_disabled = false;
    /* wait on the mutex's wait queue */
    wait(&mutex->wait_queue);
    /* back in action, so politely re-enable preemption */
    g_preemption_disabled = true;
    sti();
}

void mutex_init(mutex_t* mutex)
{
    KASSERT(mutex);
    mutex->locked = false;
    mutex->owner = NULL;
    thread_queue_clear(&mutex->wait_queue);
}

void mutex_lock(mutex_t* mutex)
{
    KASSERT(interrupts_enabled());
    KASSERT(mutex);

    disable_preemption();

    KASSERT(!mutex_held(mutex));

    while (mutex->locked) {
        mutex_wait(mutex);
    }

    mutex->locked = true;
    mutex->owner = g_current_thread;

    enable_preemption();
}

void mutex_unlock(mutex_t* mutex)
{
    KASSERT(interrupts_enabled());
    KASSERT(mutex);

    disable_preemption();

    KASSERT(mutex_held(mutex));

    mutex->locked = false;
    mutex->owner = NULL;

    if (!thread_queue_empty(&mutex->wait_queue)) {
        cli();
        wake_one(&mutex->wait_queue);
        sti();
    }

    enable_preemption();
}

bool mutex_held(mutex_t* mutex)
{
    KASSERT(mutex);
    if (mutex->locked && mutex->owner == g_current_thread) {
        return true;
    }
    return false;
}

void disable_preemption(void)
{
    g_preemption_disabled = true;
}

void enable_preemption(void)
{
    g_preemption_disabled = false;
}

bool preemption_enabled(void)
{
    return !g_preemption_disabled;
}
