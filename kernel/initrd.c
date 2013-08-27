#include "blkdev.h"
#include "initrd.h"

int ramdisk_open(block_device_t* dev)
{
    return 1;
}

int ramdisk_close(block_device_t* dev)
{
    return 1;
}

int ramdisk_read(block_device_t* dev, unsigned int addr, unsigned int bytes, void* buf)
{
    return bytes;
}

int ramdisk_write(block_device_t* dev, unsigned int addr, unsigned int bytes, void* buf)
{
    return bytes;
}

static struct block_device_ops ramdisk_block_device_ops =
{
    ramdisk_open,
    ramdisk_close,
    NULL,
    ramdisk_read,
    ramdisk_write
};

void ramdisk_init(void* start, size_t len)
{
    block_device_t* ramdisk_device = register_block_device(
            "initrd", 1, &ramdisk_block_device_ops);
}
