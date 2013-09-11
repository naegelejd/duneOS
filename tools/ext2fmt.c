#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "kernel/ext2.h"

#define DIE(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)

#define DEBUG(...) \
    do { \
        printf(__VA_ARGS__); \
    } while (0)


static void populate_superblock(struct ext2_superblock *sb,
        long image_size, long block_size);

int main(int argc, char** argv)
{
    if (argc < 2) {
        DIE("Missing filesystem size in MB\n");
    } else if (argc < 3) {
        DIE("Missing ext2 block size in KB\n");
    } else if (argc < 4) {
        DIE("Missing image name\n");
    }

    long image_size = strtol(argv[1], NULL, 0) * 1024 * 1024;
    long image_block_size = strtol(argv[2], NULL, 0) * 1024;
    char *image_name = argv[3];

    if (image_block_size % EXT2_MIN_BLOCK_SIZE != 0) {
        DIE("ext2 block size must be a multiple of %u\n", EXT2_MIN_BLOCK_SIZE);
    }

    if (remove(image_name) != 0) {
        DIE("Failed to delete existing filesystem image\n");
    }

    FILE* fp = NULL;
    if ((fp = fopen(image_name, "wb")) == NULL) {
        DIE("Failed to open filesystem image %s\n", image_name);
    }

    DEBUG("Filesystem size in bytes: %ld\n", image_size);
    DEBUG("Filesystem block size in bytes: %ld\n", image_block_size);
    DEBUG("Sizeof superblock: %lu\n", sizeof(struct ext2_superblock));
    DEBUG("Sizeof bg descriptor: %lu\n", sizeof(struct ext2_block_group_descr));
    DEBUG("Sizeof inode: %lu\n", sizeof(struct ext2_inode));

    struct ext2_superblock sb;
    populate_superblock(&sb, image_size, image_block_size);

    unsigned int n_block_groups = sb.s_blocks_count / sb.s_blocks_per_group;
    unsigned int block_group_size = sb.s_blocks_per_group * image_block_size;

    size_t block_group_descr_table_size =
            n_block_groups * sizeof(struct ext2_block_group_descr);
    struct ext2_block_group_descr *block_group_descr_table =
            malloc(block_group_descr_table_size);
    if (block_group_descr_table == NULL) {
        DIE("Failed to allocate block group descriptor table\n");
    }

    DEBUG("Seeking to first superblock\n");
    if (fseek(fp, 1024, SEEK_SET) != 0) {
        DIE("Failed to seek to first superblock\n");
    }

    DEBUG("Writing first superblock\n");
    if (fwrite(&sb, sizeof(sb), 1, fp) != 1) {
        DIE("Failed to write superblock\n");
    }

    unsigned int bg = 0;
    long fpos = 0;
    for (bg = 1; bg < n_block_groups; bg++) {
        fpos = bg * block_group_size;
        if (fseek(fp, fpos, SEEK_SET) != 0) {
            DIE("Failed to seek to first superblock\n");
        }
        if (fwrite(&sb, sizeof(sb), 1, fp) != 1) {
            DIE("Failed to write superblock\n");
        }

        fpos += image_block_size;
        if (fseek(fp, fpos, SEEK_SET) != 0) {
            DIE("Failed to seek to first superblock\n");
        }
        if (fwrite(&sb, block_group_descr_table_size, 1, fp) != 1) {
            DIE("Failed to write superblock\n");
        }

    }

    if (fclose(fp) != 0) {
        DIE("Failed to close filesystem image\n");
    }

    return EXIT_SUCCESS;
}

/* Equivalent to log(num) / log(2) or just
 * log2 if you have `math.h` */
static unsigned int logtwo(unsigned int num)
{
    unsigned int p = 0;
    while (num > 1) {
        p++;
        num /= 2;
    }

    return p;
}

static void populate_superblock(struct ext2_superblock *sb,
        long image_size, long block_size)
{
    unsigned int usable_blocks = (unsigned int)(image_size / block_size);

    sb->s_blocks_count = usable_blocks;
    sb->s_r_blocks_count = usable_blocks / 32;
    sb->s_first_data_block = block_size / EXT2_SUPERBLOCK_LOCATION;
    sb->s_log_block_size = logtwo(block_size / 1024);
    sb->s_log_frag_size = sb->s_log_block_size;
    sb->s_blocks_per_group = 8 * block_size; /* max blocks per group */
    sb->s_frags_per_group = 0;
    sb->s_inodes_per_group = 8 * block_size; /* max inodes per group */

    /* slightly out of order w/ respect to superblock definition */
    unsigned int num_block_groups = sb->s_blocks_count / sb->s_blocks_per_group;
    unsigned int block_group_descr_table_size = num_block_groups *
            sizeof(struct ext2_block_group_descr);
    unsigned int block_group_size = sb->s_blocks_per_group * block_size;

    DEBUG("Number of blocks: %u\n", sb->s_blocks_count);
    DEBUG("Number of block groups: %u\n", num_block_groups);
    DEBUG("Size of block group descriptor table: %u\n",
            block_group_descr_table_size);
    DEBUG("Size of block group: %u\n", block_group_size);

    sb->s_inodes_count = sb->s_inodes_per_group * num_block_groups;

    /* TODO: update this to account for all USED blocks:
     * super block and each copy
     * block group descriptor table and all copies
     * each inode table
     */
    sb->s_free_blocks_count = sb->s_blocks_count;

    /* free inodes - skipping first 11 inodes in each block group */
    sb->s_free_inodes_count = sb->s_inodes_count - (
            EXT2_GOOD_OLD_FIRST_INO * num_block_groups);

    sb->s_mtime = 0;
    sb->s_wtime = 0;
    sb->s_mnt_count = 0;
    sb->s_max_mnt_count = 32;   /* arbitrary... */
    sb->s_magic = EXT2_SUPER_MAGIC;
    sb->s_state = EXT2_VALID_FS;
    sb->s_errors = EXT2_ERRORS_PANIC;
    sb->s_minor_rev_level = 0;
    sb->s_lastcheck = 0;
    sb->s_checkinterval = 60 * 60 * 24 * 64; /* 64 days? */
    sb->s_creator_os = EXT2_OS_LINUX;   /* why not */
    sb->s_rev_level = 0;
    sb->s_def_resuid = EXT2_DEF_RESUID;
    sb->s_first_ino = EXT2_GOOD_OLD_FIRST_INO;
    sb->s_inode_size = EXT2_GOOD_OLD_INODE_SIZE;
    sb->s_block_group_nr = sb->s_first_data_block; /* update for copies! */
    sb->s_feature_compat = 0;   /* no features :( */
    sb->s_feature_incompat = 0;
    sb->s_feature_ro_compat = 0;

    uint32_t *s_uuid = &sb->s_uuid[0];
    *s_uuid++ = (uint32_t)random();
    *s_uuid++ = (uint32_t)random();
    *s_uuid++ = (uint32_t)random();
    *s_uuid = (uint32_t)random();

    memset(sb->s_volume_name, 0, 16);
    memset(sb->s_last_mounted, 0, 64);

    sb->s_algo_bitmap = EXT2_GZIP_ALG; /* not used */
    sb->s_prealloc_blocks = 0;
    sb->s_prealloc_dir_blocks = 0;
    memset(sb->s_journal_uuid, 0, sizeof(sb->s_journal_uuid));
    sb->s_journal_inum = 0;
    sb->s_journal_dev = 0;
    sb->s_last_orphan = 0;

    uint32_t *s_hash_seed = &sb->s_hash_seed[0];
    *s_hash_seed++ = (uint32_t)random();
    *s_hash_seed++ = (uint32_t)random();
    *s_hash_seed++ = (uint32_t)random();
    *s_hash_seed = (uint32_t)random();

    sb->s_def_hash_version = 0;
    sb->s_default_mount_options = 0;
    sb->s_first_meta_bg = 0;
}
