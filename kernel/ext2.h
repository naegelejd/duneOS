#ifndef DUNE_EXT2_H
#define DUNE_EXT2_H

#include "dune.h"
/*
 * information in this file derived from
 * http://www.nongnu.org/ext2-doc/ext2.html
 */

enum { EXT2_MIN_BLOCK_SIZE = 1024 };
enum { EXT2_SUPERBLOCK_LOCATION = 1024 };
enum { EXT2_SUPER_MAGIC = 0xEF53 };

enum ext2_s_state { EXT2_VALID_FS = 1, EXT2_ERROR_FS };
enum ext2_s_errors { EXT2_ERRORS_CONTINUE = 1, EXT2_ERRORS_RO, EXT2_ERRORS_PANIC };
enum ext2_s_creator_os { EXT2_OS_LINUX, EXT2_OS_HURD, EXT2_OS_MASIX, EXT2_OS_FREEBSD, EXT2_OS_LITES };
enum ext2_s_rev_level { EXT2_GOOD_OLD_REV, EXT2_DYNAMIC_REV };
enum ext2_s_def_resuid { EXT2_DEF_RESUID };
enum ext2_s_def_resgid { EXT2_DEF_RESGID };
enum ext2_s_first_ino { EXT2_GOOD_OLD_FIRST_INO = 11 };
enum ext2_s_inode_size { EXT2_GOOD_OLD_INODE_SIZE = 128 };

enum ext2_s_feature_compat {
    EXT2_FEATURE_COMPAT_DIR_PREALLOC = 0x01, /* block pre-allocation for new dirs */
    EXT2_FEATURE_COMPAT_IMAGIC_NODES = 0x02,
    EXT3_FEATURE_COMPAT_HAS_JOURNAL = 0x04, /* ext3 journal exists */
    EXT2_FEATURE_COMPAT_EXT_ATTR = 0x08,    /* extended inode attributes present */
    EXT2_FEATURE_COMPAT_RESIZE_INO = 0x10,  /* non-standard inode size used */
    EXT2_FEATURE_COMPAT_DIR_INDEX = 0x20    /* directory indexing (HTree) */
};

enum ext2_s_feature_incompat {
    EXT2_FEATURE_INCOMPAT_COMPRESSION = 0x01,   /* Disk/File compression is used */
    EXT2_FEATURE_INCOMPAT_FILETYPE = 0x02,
    EXT3_FEATURE_INCOMPAT_RECOVER = 0x04,
    EXT3_FEATURE_INCOMPAT_JOURNAL_DEV = 0x08,
    EXT2_FEATURE_INCOMPAT_META_BG = 0x10
};

enum ext2_s_feature_ro_compat {
    EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER = 0x01, /* sparse superblock */
    EXT2_FEATURE_RO_COMPAT_LARGE_FILE = 0x02,   /* large file support, 64-bit file size */
    EXT2_FEATURE_RO_COMPAT_BTREE_DIR = 0x04     /* binary tree sorted directories */
};

enum ext2_s_algo_bitmap {
    EXT2_LZV1_ALG = 0x01,
    EXT2_LZRW3A_ALG = 0x02,
    EXT2_GZIP_ALG = 0x04,
    EXT2_BZIP2_ALG = 0x08,
    EXT2_LZO_ALG = 0x10
};

enum ext2_i_mode {
    /* file format */
    EXT2_S_IFSOCK = 0xC000, /* socket */
    EXT2_S_IFLNK = 0xA000, /* symbolic link */
    EXT2_S_IFREG = 0x8000, /* regular file */
    EXT2_S_IFBLK = 0x6000, /* block device */
    EXT2_S_IFDIR = 0x4000, /* directory */
    EXT2_S_IFCHR = 0x2000, /* character device */
    EXT2_S_IFIFO = 0x1000, /* fifo */
    /* process execution user/group override */
    EXT2_S_ISUID = 0x0800, /* Set process User ID */
    EXT2_S_ISGID = 0x0400, /* Set process Group ID */
    EXT2_S_ISVTX = 0x0200, /* sticky bit */
    /* access rights */
    EXT2_S_IRUSR = 0x0100, /* user read */
    EXT2_S_IWUSR = 0x0080, /* user write */
    EXT2_S_IXUSR = 0x0040, /* user execute */
    EXT2_S_IRGRP = 0x0020, /* group read */
    EXT2_S_IWGRP = 0x0010, /* group write */
    EXT2_S_IXGRP = 0x0008, /* group execute */
    EXT2_S_IROTH = 0x0004, /* others read */
    EXT2_S_IWOTH = 0x0002, /* others write */
    EXT2_S_IXOTH = 0x0001 /* others execute */
};

enum ext2_i_flags {
    EXT2_SECRM_FL = 0x00000001, /* secure deletion */
    EXT2_UNRM_FL = 0x00000002, /* record for undelete */
    EXT2_COMPR_FL = 0x00000004, /* compressed file */
    EXT2_SYNC_FL = 0x00000008, /* synchronous updates */
    EXT2_IMMUTABLE_FL = 0x0000001, /*immutable file */
    EXT2_APPEND_FL = 0x0000002, /*append only */
    EXT2_NODUMP_FL = 0x0000004, /*do not dump/delete file */
    EXT2_NOATIME_FL = 0x0000008, /*do not update .i_atime */
    /* Reserved for compression usage -- */
    EXT2_DIRTY_FL = 0x0000010, /*Dirty (modified) */
    EXT2_COMPRBLK_FL = 0x0000020, /*compressed blocks */
    EXT2_NOCOMPR_FL = 0x0000040, /*access raw compressed data */
    EXT2_ECOMPR_FL = 0x0000080, /*compression error */
    /* End of compression flags -- */
    EXT2_BTREE_FL = 0x0000100, /*b-tree format directory */
    EXT2_INDEX_FL = 0x0000100, /*hash indexed directory */
    EXT2_IMAGIC_FL = 0x0000200, /*AFS directory */
    EXT3_JOURNAL_DATA_FL = 0x0000400, /*journal file data */
    EXT2_RESERVED_FL = 0x8000000, /*reserved for ext2 library */
};

enum ext2_file_type {
    EXT2_FT_UNKNOWN = 0, /* Unknown File Type */
    EXT2_FT_REG_FILE = 1, /* Regular File */
    EXT2_FT_DIR = 2, /* Directory File */
    EXT2_FT_CHRDEV = 3, /* Character Device */
    EXT2_FT_BLKDEV = 4, /* Block Device */
    EXT2_FT_FIFO = 5, /* Buffer File */
    EXT2_FT_SOCK = 6, /* Socket File */
    EXT2_FT_SYMLINK = 7 /* Symbolic Link */
};

struct ext2_superblock {
    /* total number of inodes */
    uint32_t s_inodes_count;
    /* total number of blocks */
    uint32_t s_blocks_count;
    /* total number of blocks reserved for super user */
    uint32_t s_r_blocks_count;
    /* total number of free blocks */
    uint32_t s_free_blocks_count;
    /* total number of free inodes */
    uint32_t s_free_inodes_count;
    /* ID of block containing superblock (0 or 1 for 1KB block size)*/
    uint32_t s_first_data_block;
    /* block size = 1024 << s_log_block_size) */
    uint32_t s_log_block_size;
    /* fragment size = 1024 << s_log_frag_size (neg number means shift right) */
    int32_t s_log_frag_size;
    /* total number of blocks per group */
    uint32_t s_blocks_per_group;
    /* total number of fragments per group */
    uint32_t s_frags_per_group;
    /* total number of inodes per group */
    /* == (1024 << s_log_block_size) / s_inode_size */
    uint32_t s_inodes_per_group;
    /* UNIX time of last mount */
    uint32_t s_mtime;
    /* UNIX time of last write */
    uint32_t s_wtime;
    /* number of times mounted since last run verification */
    uint16_t s_mnt_count;
    /* number of mounts allowed before full check required */
    uint16_t s_max_mnt_count;
    /* ext2 magic */
    uint16_t s_magic;
    /* filesystem state (valid/error) */
    uint16_t s_state;
    /* what the file system driver should do upon error */
    uint16_t s_errors;
    /* minor revision level identifier */
    uint16_t s_minor_rev_level;
    /* UNIX time of last filesystem check */
    uint32_t s_lastcheck;
    /* maximum UNIX time interval allowed between filesystem checks */
    uint32_t s_checkinterval;
    /* operating system creator identifier */
    uint32_t s_creator_os;
    /* revision level identifier */
    uint32_t s_rev_level;
    /* default user ID of reserved blocks */
    uint16_t s_def_resuid;
    /* default group ID of reserved blocks */
    uint16_t s_def_resgid;
    /* index to first inode usable for regular files */
    uint32_t s_first_ino;
    /* size of inode struct */
    uint16_t s_inode_size;
    /* block group number hosting this superblock */
    uint16_t s_block_group_nr;
    /* bitmask of compatible features */
    uint32_t s_feature_compat;
    /* bitmask of incompatible features */
    uint32_t s_feature_incompat;
    /* bitmask of 'read-only' features */
    uint32_t s_feature_ro_compat;
    /* volume id (unique as possible) */
    uint32_t s_uuid[4];
    /* volume name (mostly unused - ISO-Latin-1, nul terminated */
    char s_volume_name[16];
    /* directory path where file system was last mounted */
    char s_last_mounted[64];
    /* compression algorithm identifier */
    uint32_t s_algo_bitmap;
    /* number of blocks to attempt to pre-allocate for new regular files */
    uint8_t s_prealloc_blocks;
    /* number of blocks to attempt to pre-allocate for new directory */
    uint8_t s_prealloc_dir_blocks;
    /* uuid of the journal superblock */
    uint32_t s_journal_uuid[4];
    /* inode number of journal file */
    uint32_t s_journal_inum;
    /* device number of journal file */
    uint32_t s_journal_dev;
    /* inode number of first inode in list of inodes to delete */
    uint32_t s_last_orphan;
    /* seeds used for hash algorithm for directory indexing */
    uint32_t s_hash_seed[4];
    /* default hash version for directory indexing */
    uint8_t s_def_hash_version;
    /* default mount options for this file system */
    uint32_t s_default_mount_options;
    /* block group ID of first meta block group (ext3 only?) */
    uint32_t s_first_meta_bg;
};

/* The Block Group Descriptor Table is an array of block group descriptors
 * which is located at the first block following the superblock, which is
 * always the third block for 1KB block-size systems, or the second block for all
 * larger block-size systems.
 * Shadow copies are also stored with each copy of the superblock.
 * Each block group descriptor describes exactly one block group.
 * All IDs are 'absolute'.
 *
 * Block Bitmap - located at first block (or second if superblock backup is present).
 *  Each bit represents current state of block within block group (1=used, 0=free).
 *
 * Inode Bitmap - Each bit represents current state of inode within Inode Table.
 *  When Inode Table is created, all reserved inodes are marked used (first 11). */
struct ext2_block_group_descr {
    /* block ID of first block of the "block bitmap" for this group */
    uint32_t bg_block_bitmap;
    /* block ID of first block of the "inode bitmap" for this group */
    uint32_t bg_inode_bitmap;
    /* block ID of first block of the "inode table" for this group */
    uint32_t bg_inode_table;
    /* total number of free blocks for this group */
    uint16_t bg_free_blocks_count;
    /* total number of free inodes for this group */
    uint16_t bg_free_inodes_count;
    /* total number of inodes allocated to directories for this group */
    uint16_t bg_used_dirs_count;
    /* padding */
    uint16_t bg_pad;
    /* reserved for future revisions */
    uint8_t bg_reserved[12];
};

/* The Inode Table is an array of inodes. One Inode Table exists for each
 * Block Group. There are `s_inodes_per_group` inodes per table.
 * In revision 0 of EXT2, the first 11 inodes are reserved.
 * In revision 1+, the number of reserved inodes is specified by `s_first_ino`.
 *
 * Locating inodes:
 *      (inode 1 is first inode in inode table)
 *
 * block_group = (inode - 1) / s_inodes_per_group
 *
 * local_inode_index = (inode - 1) % s_inodes_per_group
 */
struct ext2_inode {
    /* format of the described file and access rights */
    uint16_t i_mode;
    /* user id associated with file */
    uint16_t i_uid;
    /* size of the file in bytes. In rev 1+, upper 32-bits are in `i_dir_acl` */
    uint32_t i_size;
    /* number of seconds since Jan 1 1970 - last time this inode was accessed */
    uint32_t i_atime;
    /* number of seconds since Jan 1 1970 - when this inode was created */
    uint32_t i_ctime;
    /* number of seconds since Jan 1 1970 - when this inode was modified */
    uint32_t i_mtime;
    /* number of seconds since Jan 1 1970 - when this inode was deleted */
    uint32_t i_dtime;
    /* value of POSIX group having access to this file */
    uint16_t i_gid;
    /* how many times this inode is linked to (1 on creation) (hard links only) */
    uint16_t i_links_count;
    /* number of 512-byte blocks reserved to contain data of this inode */
    uint32_t i_blocks;
    /* how EXT2 implementation should behave when accessing data for this inode */
    uint32_t i_flags;
    /* OS-dependent value */
    uint32_t i_osd1;
    /* block numbers pointing to data blocks for this inode.
     * first 12 are direct blocks
     * 13th is block number of first indirect block,
     * 14th is block number of doubly-indirect block
     * 15th is block number of triply-indirect block */
    uint32_t i_block[15];
    /* file version (NFS) */
    uint32_t i_generation;
    /* block number containing extended attributes (0 in revision 0) */
    uint32_t i_file_acl;
    /* 0 in revision 0, high 32-bits of 64-bit file size in revision 1+ */
    uint32_t i_dir_acl;
    /* location of file fragment (unused) */
    uint32_t i_faddr;
    /* OS-dependent */
    uint8_t i_osd2[12];
};

struct ext2_dir_entry {
    /* inode number of the file entry */
    uint32_t inode;
    /* displacement to next directory entry from start of this entry
     * must be 4-byte aligned and cannot span data blocks */
    uint16_t rec_len;
    /* number of bytes of character data contained in the name */
    uint8_t name_len;
    /* file type identifier */
    uint8_t file_type;
    /* entry name (0-255 bytes, ISO-Latin-1, nul terminated) */
    /* char* name; */
};

#endif /* DUNE_EXT2_H */
