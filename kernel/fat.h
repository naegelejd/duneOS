#ifndef DUNE_FAT_H
#define DUNE_FAT_H

#include "dune.h"

/* EBPB volume_id is typically constructed by:
 *
 * INT 21h/AH=2Ah (get system date) sets DX and CX (1)
 * INT 21h/AH=2Ch (get system time) sets DX and CX (2)
 *
 * volume_id (high) = DX1 + DX2
 * volume_id (low) = CX1 + CX2
 */

struct extended_bios_parameter_block {
    /* 0x0 - Base BPB */
    uint16_t bytes_per_sector;  /* normally 512 */
    uint8_t sectors_per_cluster;    /* any power of 2 from 1-128 */
    uint16_t num_reserved_sectors;  /* num sectors before first FAT (32 for fat32) */
    uint8_t num_fats;               /* num file allocation tables (2) (1 for ramdisk) */
    uint16_t max_root_dir_entries;  /* 0 for FAT32 - root dir stored in normal cluster */
    uint16_t total_logical_sectors_short;   /* if 0, use 4-byte value at offset 0x20 */
    uint8_t media_descriptor;   /* meh - use 0xF8 or 0xFA for ramdisk */
    uint16_t logical_sectors_per_fat_short;   /* 0 for FAT32! */

    /* 0x0D - DOS 3.31 BPB extension */
    uint16_t physical_sectors_per_track;    /* 0 */
    uint16_t num_disk_heads;                /* 0 */
    uint32_t num_hidden_sectors;            /* 0 */
    uint32_t total_logical_sectors;         /* used if num sectors > 65535 */

    /* 0x19 - FAT32 Extended BPB */
    uint32_t logical_sectors_per_fat;   /* used for FAT32 */
    uint8_t drive_description;  /* 0x8F - char 'O' and bit 13 cleared for LBA access */
    uint8_t mirroring_flags;    /* bit 7 - set: all FATS mirrored, cleared: bits 3-0 identify active FAT */
    uint16_t version;           /* 0x00 */
    uint32_t cluster_start_number;  /* cluster number of starting root directory (typically 2) */
    uint16_t fs_information_sector;     /* logical sector num of FS information sector (typically 1) */
    uint16_t boot_copy_sector;  /* first logical sector of copy of 3 FAT32 boot sectors (typically 6) */
    uint8_t reserved[12];       /* 0x00 */

    /* 0x35 - FAT12/16 Extended BPB */
    uint8_t physical_drive_number;  /* 0x0 for removable, 0x80 for fixed disk */
    uint8_t unused;
    uint8_t extended_boot_sig;  /* should be 0x29 */
    uint32_t volume_id;         /* essentially date + time (see above) */
    uint8_t partition_volume_label[11];     /* any name - padded with spaces */
    uint8_t file_system_type[8];    /* FAT32 padded with spaces */
};

struct filesystem_information_sector {
    uint32_t fs_info_sig0;      /* = 0x52, 0x52, 0x61, 0x41 = "RRaA" */
    uint8_t reserved0[480];     /* set to zero */
    uint32_t num_free_clusters; /* set to 0xFFFFFFFF on format, updated by OS */
    uint32_t last_allocated_cluster;    /* set to 0xFFFFFFFF on format, updated by OS */
    uint8_t reserved1[12];      /* set to zero */
    uint32_t fs_info_sig1;      /* = 0x0, 0x0, 0x55, 0xAA */
};

struct volume_boot_record {
    uint8_t jump[3];       /* short JMP + NOP on x86 */
    uint8_t oem_name[8];   /* padded with spaces - "mkdosfs " */
    /* 0x0B */
    struct bios_parameter_block;
    /* 0x1FD */
    uint8_t physical_drive_number;     /* not used - ZERO */
    /* 0x1FE */
    uint8_t boot_sector_sig[2];    /* 0x55 0xAA */
};

#endif /* DUNE_FAT_H */
