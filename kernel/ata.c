#include "ata.h"
#include "io.h"
#include "print.h"
#include "thread.h"

enum { ATA_MASTER, ATA_SLAVE };
enum { IDE_ATA, IDE_ATAPI };
enum { ATA_PRIMARY, ATA_SECONDARY };
enum { ATA_READ, ATA_WRITE };

enum { ATAPI_CMD_READ = 0xA8, ATAPI_CMD_EJECT = 0x1B };

enum {
    ATA_REG_DATA        = 0x00,
    ATA_REG_ERROR       = 0x01,
    ATA_REG_FEATURES    = 0x01,
    ATA_REG_SECCOUNT0   = 0x02,
    ATA_REG_LBA0        = 0x03,
    ATA_REG_LBA1        = 0x04,
    ATA_REG_LBA2        = 0x05,
    ATA_REG_HDDEVSEL    = 0x06,
    ATA_REG_COMMAND     = 0x07,
    ATA_REG_STATUS      = 0x07,
    ATA_REG_SECCOUNT1   = 0x08,
    ATA_REG_LBA3        = 0x09,
    ATA_REG_LBA4        = 0x0A,
    ATA_REG_LBA5        = 0x0B,
    ATA_REG_CONTROL     = 0x0C,
    ATA_REG_ALTSTATUS   = 0x0C,
    ATA_REG_DEVADDRESS  = 0x0D,

};

enum {
    ATA_IDENT_DEVICETYPE = 0,
    ATA_IDENT_CYLINDERS = 2,
    ATA_IDENT_HEADS = 6,
    ATA_IDENT_SECTORS = 12,
    ATA_IDENT_SERIAL = 20,
    ATA_IDENT_MODEL = 54,
    ATA_IDENT_CAPABILITIES = 98,
    ATA_IDENT_FIELDVALID = 106,
    ATA_IDENT_MAX_LBA = 120,
    ATA_IDENT_COMMANDSETS = 164,
    ATA_IDENT_MAX_LBA_EXT = 200,
};

enum {
    ATA_CMD_READ_PIO = 0x20,
    ATA_CMD_READ_PIO_EXT = 0x24,
    ATA_CMD_READ_DMA = 0xC8,
    ATA_CMD_READ_DMA_EXT = 0x25,
    ATA_CMD_WRITE_PIO = 0x30,
    ATA_CMD_WRITE_PIO_EXT = 0x34,
    ATA_CMD_WRITE_DMA = 0xCA,
    ATA_CMD_WRITE_DMA_EXT = 0x35,
    ATA_CMD_CACHE_FLUSH = 0xE7,
    ATA_CMD_CACHE_FLUSH_EXT = 0xEA,
    ATA_CMD_PACKET = 0xA0,
    ATA_CMD_IDENTIFY_PACKET = 0xA1,
    ATA_CMD_IDENTIFY = 0xEC,
};

enum {
    ATA_ER_BBK = 0x80,
    ATA_ER_UNC = 0x40,
    ATA_ER_MC = 0x20,
    ATA_ER_IDNF = 0x10,
    ATA_ER_MCR = 0x08,
    ATA_ER_ABRT = 0x04,
    ATA_ER_TK0NF = 0x02,
    ATA_ER_AMNF = 0x01,
};

enum {
    ATA_SR_BSY = 0x80,
    ATA_SR_DRDY = 0x40,
    ATA_SR_DF = 0x20,
    ATA_SR_DSC = 0x10,
    ATA_SR_DRQ = 0x08,
    ATA_SR_CORR = 0x04,
    ATA_SR_IDX = 0x02,
    ATA_SR_ERR = 0x01,
};

struct channel {
    uint16_t base;      /* I/O Base */
    uint16_t ctrl;      /* Control base */
    uint16_t bmide;     /* Bus Master IDE */
    uint8_t nIEN;       /* no interrupt */
} channels[2];

struct ide_device {
    uint8_t reserved;       /* 0 (Empty) or 1 (This Drive really exists) */
    uint8_t channel;        /* 0 (Primary Channel) or 1 (Secondary Channel) */
    uint8_t drive;          /* 0 (Master Drive) or 1 (Slave Drive) */
    uint16_t type;          /* 0: ATA, 1:ATAPI */
    uint16_t sign;          /* Drive Signature */
    uint16_t capabilities;  /* Features */
    uint32_t commandsets;   /* Command Sets Supported */
    uint32_t size;          /* Size in Sectors */
    char model[41];         /* Model in string */
} ide_devices[4];

uint8_t ide_buffer[2048] = {0};
static uint8_t ide_irq_invoked = 0;
static uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void ide_write(uint8_t channel, uint8_t reg, uint8_t data)
{
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    }
    if (reg < 0x08) {
        outportb(channels[channel].base + reg - 0x00, data);
    } else if (reg < 0x0C) {
        outportb(channels[channel].base + reg - 0x06, data);
    } else if (reg < 0x0E) {
        outportb(channels[channel].ctrl + reg - 0x0A, data);
    } else if (reg < 0x16) {
        outportb(channels[channel].bmide + reg - 0x0E, data);
    }
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    }
}

static uint8_t ide_read(uint8_t channel, uint8_t reg)
{
    uint8_t result;
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    }
    if (reg < 0x08) {
        result = inportb(channels[channel].base + reg - 0x00);
    } else if (reg < 0x0C) {
        result = inportb(channels[channel].base + reg - 0x06);
    } else if (reg < 0x0E) {
        result = inportb(channels[channel].ctrl + reg - 0x0A);
    } else if (reg < 0x16) {
        result = inportb(channels[channel].bmide + reg - 0x0E);
    }
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    }
    return result;

}

static void ide_read_buffer(uint8_t channel, uint8_t reg, uintptr_t buffer, unsigned int words)
{
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    }
    /* asm volatile ("pushw %es; pushw %ax; movw %ds, %ax; movw %ax, %es; popw %ax;"); */
    if (reg < 0x08) {
        inportsw(channels[channel].base + reg - 0x00, buffer, words);
    } else if (reg < 0x0C) {
        inportsw(channels[channel].base + reg - 0x06, buffer, words);
    } else if (reg < 0x0E) {
        inportsw(channels[channel].ctrl + reg - 0x0A, buffer, words);
    } else if (reg < 0x16) {
        inportsw(channels[channel].bmide + reg - 0x0E, buffer, words);
    }
    /* asm volatile ("popw %es;"); */
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    }
}

void ide_initialize(uint32_t bar0, uint32_t bar1, uint32_t bar2,
        uint32_t bar3, uint32_t bar4)
{
    /* Detect I/O Ports which interface IDE Controller */
    channels[ATA_PRIMARY].base = (bar0 &= 0xFFFFFFFC) + 0x1F0 * (!bar0);
    channels[ATA_PRIMARY].ctrl = (bar1 &= 0xFFFFFFFC) + 0x3F4 * (!bar1);
    channels[ATA_SECONDARY].base = (bar2 &= 0xFFFFFFFC) + 0x170 * (!bar2);
    channels[ATA_SECONDARY].ctrl = (bar3 &= 0xFFFFFFFC) + 0x374 * (!bar3);
    channels[ATA_PRIMARY ].bmide = (bar4 &= 0xFFFFFFFC) + 0;    /* Bus Master IDE */
    channels[ATA_SECONDARY].bmide = (bar4 &= 0xFFFFFFFC) + 8;   /* Bus Master IDE */

    DEBUGF("%X %X %X\n", channels[ATA_PRIMARY].base, channels[ATA_PRIMARY].ctrl, channels[ATA_PRIMARY].bmide);
    DEBUGF("%X %X %X\n", channels[ATA_SECONDARY].base, channels[ATA_SECONDARY].ctrl, channels[ATA_SECONDARY].bmide);

    /* Disable IRQs */
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
    DEBUG("disabled IRQs\n");

    int count = 0;
    int chan = 0, drive = 0;
    /* Detect ATA-ATAPI Devices */
    for (chan = 0; chan < 2; chan++) {
        for (drive = 0; drive < 2; drive++) {
            bool err = false;
            uint8_t type = IDE_ATA, status;

            ide_devices[count].reserved = 0;    /* Assuming that no drive here. */

            /* Select Drive */
            DEBUGF("selecting channel %d, drive %d\n", chan, drive);
            ide_write(chan, ATA_REG_HDDEVSEL, 0xA0 | (drive << 4));
            sleep(1);

            /* Send ATA Identify Command */
            DEBUG("sending ATA IDENTIFY command\n");
            ide_write(chan, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            sleep(1);

            /* Polling */
            if (!(ide_read(chan, ATA_REG_STATUS))) {
                DEBUG("no device found\n");
                continue;   /* If Status = 0, No Device */
            } else {
                DEBUG("device found!\n");
            };

            while (true) {
                status = ide_read(chan, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR)) {
                    err = true;
                    DEBUG("not an ATA device\n");
                    break; /* If Err, Device is not ATA */
                }
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
                    DEBUG("found ATA device\n");
                    break;  /* Everything is right */
                }
            }

            /* Probe for ATAPI Devices */

            if (err) {
                uint8_t cl = ide_read(chan, ATA_REG_LBA1);
                uint8_t ch = ide_read(chan, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB) {
                    type = IDE_ATAPI;
                } else if (cl == 0x69 && ch == 0x96) {
                    type = IDE_ATAPI;
                } else {
                    DEBUG("not an ATAPI device either\n");
                    continue;   /* Unknown Type (not a device) */
                }

                ide_write(chan, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                sleep(1);
                DEBUG("found ATAPI device\n");
            }

            /* Read Identification Space of the Device */
            ide_read_buffer(chan, ATA_REG_DATA, (uint32_t)ide_buffer, 256);
            DEBUG("read identification space\n");

            /* Read Device Parameters */
            ide_devices[count].reserved = 1;
            ide_devices[count].type = type;
            ide_devices[count].channel = chan;
            ide_devices[count].drive = drive;
            ide_devices[count].sign = ((uint16_t *)(ide_buffer + ATA_IDENT_DEVICETYPE))[0];
            ide_devices[count].capabilities = ((uint16_t *)(ide_buffer + ATA_IDENT_CAPABILITIES))[0];
            ide_devices[count].commandsets = ((uint32_t *)(ide_buffer + ATA_IDENT_COMMANDSETS))[0];

            /* Get Size */
            if (ide_devices[count].commandsets & (1 << 26)) {
                /* Device uses 48-Bit Addressing */
                ide_devices[count].size = ((uint32_t *)(ide_buffer + ATA_IDENT_MAX_LBA_EXT))[0];
                /* In a 32-Bit operating system the last word is ignored */
            } else {
                /* Device uses CHS or 28-bit Addressing */
                ide_devices[count].size = ((uint32_t *)(ide_buffer + ATA_IDENT_MAX_LBA))[0];
            }

            /* String indicates model of device */
            int k = 0;
            for (k = 0; k < 40; k += 2) {
                ide_devices[count].model[k] = ide_buffer[k + 1 + ATA_IDENT_MODEL];
                ide_devices[count].model[k + 1] = ide_buffer[k + ATA_IDENT_MODEL];
            }
            ide_devices[count].model[40] = 0; /* append nul terminator */

            count++;
        }
    }

    /* Print Summary */
    int i = 0;
    for (i = 0; i < 4; i++) {
        if (ide_devices[i].reserved == 1) {
            DEBUGF("Found %s Drive %dGB - %s\n",
                    (const char *[]){"ATA", "ATAPI"}[ide_devices[i].type], /* Type */
                    ide_devices[i].size / 1024 / 1024 / 2, /* Size */
                    ide_devices[i].model);
            /* int j = 0; */
            /* for (j = 0; j < 40; j++) { */
            /*     DEBUGF("%d, ", ide_devices[i].model[j]); */
            /* } */
            /* DEBUG("\n"); */
        }
    }
}
