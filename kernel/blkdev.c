#include "string.h"
#include "mem.h"
#include "blkdev.h"

static mutex_t block_device_lock;

static block_device_t* all_devices_head;
static block_device_t* all_devices_tail;

static void all_devices_add(block_device_t* dev)
{
    if (!all_devices_head) {
        all_devices_head = dev;
    } else if (all_devices_tail) {
        all_devices_tail->next = dev;
    }
    dev->next = NULL;
    all_devices_tail = dev;
}

static void all_devices_remove(block_device_t* dev)
{
    block_device_t** d = &all_devices_head;
    while (*d) {
        if (dev == *d) {
            *d = (*d)->next;
        }
        d = &(*d)->next;
    }
}

block_device_t* register_block_device(
        const char *name,
        unsigned int blocksize,
        struct block_device_ops* ops)
{
    static unsigned int block_device_ids = 0;
    KASSERT(name);
    KASSERT(ops);

    block_device_t* dev = malloc(sizeof(*dev));
    if (!dev) {
        dbgprintf("Failed to allocate mem for block device\n");
        return NULL;
    }

    strncpy(dev->name, name, MAX_BLOCK_DEV_NAME);
    dev->id = block_device_ids++;
    dev->blocksize = blocksize;
    dev->ops = ops;

    dev->wait_queue = malloc(sizeof(*dev->wait_queue));
    if (!dev->wait_queue) {
        dbgprintf("Failed to allocate mem for block device wait queue\n");
        return NULL;
    }
    dev->wait_queue->head = NULL;
    dev->wait_queue->tail = NULL;

    dev->request_list_head = NULL;

    mutex_lock(&block_device_lock);
    all_devices_add(dev);
    mutex_unlock(&block_device_lock);

    return dev;
}

block_device_t* block_device_open(const unsigned int id)
{
    block_device_t* dev = all_devices_head;
    while (dev) {
        if (dev->id == id && dev->ops->open) {
            int ecode = dev->ops->open(dev);
            return dev;
        }
    }

    return NULL;
}

void block_device_close(block_device_t* dev)
{
    KASSERT(dev);

    if (dev->ops->close) {
        int ecode = dev->ops->close(dev);
    }
}

void block_device_read(block_device_t* dev, unsigned int start_block,
        unsigned int block_count, void* buffer)
{

}

void block_device_write(block_device_t* dev, unsigned int start_block,
        unsigned int block_count, void* buffer)
{

}
