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

struct thread_queue {
    struct kthread* head;
    struct kthread* tail;
};
typedef struct thread_queue thread_queue_t;

enum { MAX_TLOCAL_KEYS = 128 };

struct kthread {
    uint32_t esp;
    volatile uint32_t num_ticks;
    int priority;

    void* stack_page;
    struct user_context* ucontext;
    struct kthread* owner;
    int refcount;

    /* join()-related members */
    bool alive;
    thread_queue_t* joinq;
    int exitcode;

    /* kernel thread ID and process ID */
    unsigned int id;

    /* link to threads in current queue */
    struct kthread* thread_queue_prev;
    struct kthread* thread_queue_next;

    /* link to all threads in system */
    struct kthread* all_threads_prev;
    struct kthread* all_threads_next;

    /* array of pointers to thread-local data */
    const void* tlocal_data[MAX_TLOCAL_KEYS];
};
typedef struct kthread thread_t;

/* Thread start functions must match this signature. */
typedef void (*thread_start_func_t)(uint32_t arg);

/* thread-local data */
typedef void (*tlocal_destructor_t)(void *);
typedef unsigned int tlocal_key_t;


void wait(thread_queue_t* wait_queue);
void wake_up(thread_queue_t* wait_queue);
void wake_up_one(thread_queue_t* wait_queue);

void yield(void);
void exit(int exit_code) __attribute__ ((noreturn));
void schedule(void);



#endif /* DUNE_THREAD_H */
