#include "string.h"
#include "int.h"
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

static void block_device_push_request(
        block_device_t* dev,
        block_request_t* request)
{
    cli();

    DEBUG("Pushing block device request\n");
    if (dev->request_list_head == NULL) {
        dev->request_list_head = request;
    } else if (dev->request_list_tail != NULL) {
        dev->request_list_tail->next = request;
    }
    request->next = NULL;
    dev->request_list_tail = request;

    DEBUG("Waking block device requestee(s)\n");
    wake_all(dev->wait_queue);
    sti();

    cli();
    while (request->state == BLOCK_REQUEST_PENDING) {
        DEBUG("Block device request pending\n");
        wait(&request->wait_queue);
    }
    sti();
}

block_device_t* register_block_device(
        const char *name,
        unsigned int blocksize,
        void* driver_data,
        struct block_device_ops* ops)
{
    static unsigned int block_device_ids = 0;
    KASSERT(name);
    KASSERT(ops);

    block_device_t* dev = malloc(sizeof(*dev));
    if (!dev) {
        DEBUG("Failed to allocate mem for block device\n");
        return NULL;
    }

    strncpy(dev->name, name, MAX_BLOCK_DEV_NAME);
    dev->id = block_device_ids++;
    dev->blocksize = blocksize;
    dev->driver_data = driver_data;
    dev->ops = ops;

    dev->wait_queue = malloc(sizeof(*dev->wait_queue));
    if (!dev->wait_queue) {
        DEBUG("Failed to allocate mem for block device wait queue\n");
        return NULL;
    }
    dev->wait_queue->head = NULL;
    dev->wait_queue->tail = NULL;

    dev->request_list_head = NULL;
    dev->request_list_tail = NULL;

    mutex_lock(&block_device_lock);
    all_devices_add(dev);
    mutex_unlock(&block_device_lock);

    return dev;
}

block_device_t* block_device_open(const unsigned int id)
{
    mutex_lock(&block_device_lock);
    block_device_t* dev = all_devices_head;
    while (dev) {
        if (dev->id == id) {
            break;
        }
    }

    if (!dev) {
        return NULL;
    }

    if (dev->ops->open) {
        int ecode = dev->ops->open(dev);
        KASSERT(ecode == 0);
    }

    mutex_unlock(&block_device_lock);

    return dev;
}

void block_device_close(block_device_t* dev)
{
    KASSERT(dev);

    if (dev->ops->close) {
        int ecode = dev->ops->close(dev);
        KASSERT(ecode == 0);
    }
}

block_request_t* block_device_pop_request(block_device_t* dev)
{
    cli();
    while (dev->request_list_head == NULL) {
        DEBUG("No block requests... device waiting\n");
        wait(dev->wait_queue);
    }

    DEBUG("Popping block request\n");
    block_request_t* request = dev->request_list_head;

    /* actually pop request from linked list */
    if (dev->request_list_head == dev->request_list_tail) {
        dev->request_list_head = NULL;
        dev->request_list_tail = NULL;
    } else {
        dev->request_list_head = request->next;
    }

    sti();

    return request;
}

void notify_requester(block_request_t* request, int state, int error)
{
    cli();
    request->state = state;
    request->ecode = error;
    DEBUG("Block request completed, waking requester\n");
    wake_all(&request->wait_queue);
    sti();
}

static void init_request(block_request_t* request, block_device_t* dev,
        int type, unsigned int start, unsigned int count, void* buffer)
{
    request->type = type;
    request->state = BLOCK_REQUEST_PENDING;
    request->block_number = start;
    request->block_count = count;
    request->device = dev;
    request->buffer = buffer;
    thread_queue_clear(&request->wait_queue);
}

void block_device_read(block_device_t* dev, unsigned int start_block,
        unsigned int block_count, void* buffer)
{
    KASSERT(block_count);
    KASSERT(buffer);

    block_request_t request;
    init_request(&request, dev, BLOCK_REQUEST_READ,
            start_block, block_count, buffer);
    block_device_push_request(dev, &request);
}

void block_device_write(block_device_t* dev, unsigned int start_block,
        unsigned int block_count, void* buffer)
{
    KASSERT(block_count);
    KASSERT(buffer);

    block_request_t request;
    init_request(&request, dev, BLOCK_REQUEST_WRITE,
            start_block, block_count, buffer);
    block_device_push_request(dev, &request);
}
