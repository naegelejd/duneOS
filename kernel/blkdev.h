#ifndef DUNE_BLKDEV_H
#define DUNE_BLKDEV_H

#include "thread.h"

enum { MAX_BLOCK_DEV_NAME = 64 };
enum { BLOCK_SIZE = 512 };

struct block_device_ops;

struct block_device {
    char name[MAX_BLOCK_DEV_NAME];
    //unsigned int blocksize;

    thread_queue_t* wait_queue;

    struct block_device *next;
};
typedef struct block_device blk_dev_t;

struct block_device_ops {
    int (*open)(blk_dev_t* dev);
    int (*close)(blk_dev_t* dev);
    int (*stat)(blk_dev_t* dev);
    int (*read)(blk_dev_t* dev, unsigned int blocknum, void* buf);
    int (*write)(blk_dev_t* dev, unsigned int blocknum, void* buf);
};

void register_block_device();

#endif /* DUNE_BLKDEV_H */
