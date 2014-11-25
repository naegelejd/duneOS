#include "string.h"
#include "mem.h"
#include "blkdev.h"
#include "initrd.h"

block_device_t* ramdisk_device = NULL;

static int test_ramdisk(unsigned int nbytes)
{
    /* dumb test */
    char* buffer1 = malloc(nbytes);
    char* buffer2 = malloc(nbytes);
    if (!buffer1 || !buffer2) {
        kprintf("Malloc error\n");
        khalt();
    }
    memset(buffer1, 0, nbytes);
    memset(buffer2, 0, nbytes);

    block_device_read(ramdisk_device, 0, nbytes, buffer1);

    unsigned int i = 0;
    for (i = 0; i < nbytes; i++) {
        buffer1[i] /= 2;
    }

    block_device_write(ramdisk_device, 0, nbytes, buffer1);
    block_device_read(ramdisk_device, 0, nbytes, buffer2);

    return memcmp(buffer1, buffer2, nbytes);
}

int ramdisk_open(block_device_t* dev)
{
    (void)dev;
    return 0;
}

int ramdisk_close(block_device_t* dev)
{
    (void)dev;
    return 0;
}

int ramdisk_read(block_device_t* dev, unsigned int offset,
        unsigned int bytes, void* buf)
{
    struct ramdisk* rd = (struct ramdisk*)dev->driver_data;
    uintptr_t start = rd->start_addr + offset;
    if (start + bytes > rd->end_addr) {
        return -1;
    }

    memcpy(buf, (void*)start, bytes);
    return bytes;
}

int ramdisk_write(block_device_t* dev, unsigned int offset,
        unsigned int bytes, void* buf)
{
    struct ramdisk* rd = (struct ramdisk*)dev->driver_data;
    uintptr_t start = rd->start_addr + offset;
    if (start + bytes > rd->end_addr) {
        return -1;
    }

    memcpy((void*)start, buf, bytes);
    return bytes;
}

void handle_ramdisk_requests(uint32_t arg)
{
    (void)arg;
    while (true) {
        int rc = 0;
        block_request_t* request = block_device_pop_request(ramdisk_device);

        if (request->type == BLOCK_REQUEST_READ) {
            rc = ramdisk_read(request->device, request->block_number,
                    request->block_count, request->buffer);
        } else if (request->type == BLOCK_REQUEST_WRITE) {
            rc = ramdisk_write(request->device, request->block_number,
                    request->block_count, request->buffer);
        } else {
            KASSERT(false);
        }

        notify_requester(request, BLOCK_REQUEST_COMPLETE, rc);
    }
}

static struct block_device_ops ramdisk_block_device_ops =
{
    ramdisk_open,
    ramdisk_close,
    NULL,
};

block_device_t* ramdisk_init(uintptr_t start, size_t len)
{
    static struct ramdisk ramdisk;
    ramdisk.start_addr = start;
    ramdisk.end_addr = start + len;
    ramdisk.length = len;

    ramdisk_device = register_block_device(
            "initrd", 1, (void*)&ramdisk, &ramdisk_block_device_ops);

    spawn_thread(handle_ramdisk_requests, 0, PRIORITY_NORMAL, true, false);

    unsigned int nbytes = len / 7;
    KASSERT(test_ramdisk(nbytes) == 0);

    return ramdisk_device;
}
