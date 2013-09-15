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

#ifndef DUNE_THREAD_H
#define DUNE_THREAD_H

#include "dune.h"

/* forward declaration for now */
struct user_context;

/* thread-local data */
enum { MAX_TLOCAL_KEYS = 128 };
typedef void (*tlocal_destructor_t)(void *);
typedef unsigned int tlocal_key_t;

/* global quantum (number of ticks before current thread yields) */
enum { THREAD_QUANTUM = 4 };

enum priority {
    PRIORITY_IDLE = 0,
    PRIORITY_USER = 1,
    PRIORITY_LOW  = 2,
    PRIORITY_NORMAL = 5,
    PRIORITY_HIGH = 10
};
typedef enum priority priority_t;

/* thread queues/lists */
struct thread_queue {
    struct thread* head;
    struct thread* tail;
};
typedef struct thread_queue thread_queue_t;


/* kernel thread definition */
struct thread {
    uint32_t esp;
    volatile uint32_t num_ticks;
    priority_t priority;

    void* stack_page;
    struct user_context* ucontext;
    struct thread* owner;
    int refcount;

    /* sleep */
    uint32_t sleep_until;

    /* join()-related members */
    bool alive;
    struct thread_queue join_queue;
    int exit_code;

    /* kernel thread ID and process ID */
    unsigned int id;

    /* link to next thread in current queue */
    struct thread* queue_next;

    /* link to all threads in system */
    struct thread* list_next;

    /* array of pointers to thread-local data */
    const void* tlocal_data[MAX_TLOCAL_KEYS];
};
typedef struct thread thread_t;

/* Thread start functions must match this signature. */
typedef void (*thread_start_func_t)(uint32_t arg);

struct mutex {
    bool locked;
    thread_t* owner;
    thread_queue_t wait_queue;
};
typedef struct mutex mutex_t;


int join(thread_t* thread);
void sleep(unsigned int ticks);
void yield(void);
//void exit(int exit_code) __attribute__ ((noreturn));
void exit(int exit_code);

void wait(thread_queue_t* wait_queue);
void make_runnable(thread_t* thread);
void make_runnable_atomic(thread_t* thread);

thread_t* start_kernel_thread(thread_start_func_t start_function,
        uint32_t arg, priority_t priority, bool detached);

void schedule(void);
void scheduler_init();

void dump_thread_info(thread_t*);
void dump_all_threads_list(void);

void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);
bool mutex_held(mutex_t* mutex);

void thread_queue_clear(thread_queue_t* queue);
bool thread_queue_empty(thread_queue_t* queue);
void wake_all(thread_queue_t* wait_queue);
void wake_one(thread_queue_t* wait_queue);

thread_t* get_current_thread(void);

void disable_preemption(void);
void enable_preemption(void);
bool preemption_enabled(void);

/* Thread-local data functions */
bool tlocal_create(tlocal_key_t* key, tlocal_destructor_t destructor);
void tlocal_set(tlocal_key_t key, const void* data);
void* tlocal_get(tlocal_key_t key);

#endif /* DUNE_THREAD_H */
