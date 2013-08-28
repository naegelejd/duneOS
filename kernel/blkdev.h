#ifndef DUNE_BLKDEV_H
#define DUNE_BLKDEV_H

#include "thread.h"

enum { MAX_BLOCK_DEV_NAME = 64 };
enum { BLOCK_SIZE = 512 };

struct block_device_ops;
struct block_request;

struct block_device {
    unsigned int id;            /* unique block device ID */
    unsigned int blocksize;     /* size in bytes of one block */
    char name[MAX_BLOCK_DEV_NAME];  /* name of device */
    void* driver_data;          /* implementation-specific data */
    thread_queue_t* wait_queue; /* queue for request fulfilling thread */
    struct block_device* next;  /* next in linked list */
    struct block_request* request_list_head;
    struct block_request* request_list_tail;
    struct block_device_ops* ops;
};
typedef struct block_device block_device_t;

enum request_type { BLOCK_REQUEST_READ, BLOCK_REQUEST_WRITE };
enum request_state { BLOCK_REQUEST_PENDING, BLOCK_REQUEST_COMPLETE };
enum request_error { BLOCK_REQUEST_FAIL };

struct block_request {
    int type;                   /* read/write */
    int state;                  /* request's current state */
    int ecode;                  /* potential error code */
    unsigned int block_number;  /* starting block number */
    unsigned int block_count;   /* number of blocks to read/write */
    block_device_t* device;     /* device on which to read/write */
    void* buffer;               /* block read/write source/destination */
    thread_queue_t wait_queue;  /* queue for the waiting requester */
    struct block_request* next; /* next in linked list */
};
typedef struct block_request block_request_t;

struct block_device_ops {
    int (*open)(block_device_t* dev);
    int (*close)(block_device_t* dev);
    int (*stat)(block_device_t* dev);
    /* int (*read)(block_device_t* dev, unsigned int blocknum, */
    /*         unsigned int block_count, void* buf); */
    /* int (*write)(block_device_t* dev, unsigned int blocknum, */
    /*         unsigned int block_count, void* buf); */
};

block_device_t* register_block_device(
        const char *name,
        unsigned int blocksize,
        void* driver_data,
        struct block_device_ops* ops);

block_device_t* open_block_device(const char* name);
void close_block_device(block_device_t* dev);
void block_device_read(block_device_t* dev, unsigned int start_block,
        unsigned int block_count, void* buffer);
void block_device_write(block_device_t* dev, unsigned int start_block,
        unsigned int block_count, void* buffer);

void notify_requester(block_request_t* request, int state, int error);
block_request_t* block_device_pop_request(
        block_device_t* dev);
        /* , thread_queue_t* requestee_wait_queue); */

#endif /* DUNE_BLKDEV_H */
