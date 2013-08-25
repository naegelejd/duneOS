#ifndef DUNE_VFS_H
#define DUNE_VFS_H

struct inode {
    char name[128];
    uint32_t length;
    uint32_t ino;
    uint32_t devno;

    uint32_t atime;     /* accessed time */
    uint32_t mtime;     /* modified time */
    uint32_t ctime;     /* created time */

    struct inode_ops* ops;
    struct inode *alias;
};
typedef struct inode inode_t;

struct file_ops {
    int (*stat)(inode_t*);
    int (*open)(inode_t*);
    int (*close)(inode_t*);
    int (*read)(inode_t*);
    int (*write)(inode_t*);
};

struct inode_ops {
    struct file_ops* file_ops;
    int (*mkdir)(inode_t*);
}

struct dirent {
    uint32_t ino;
    char name[128];
}

#endif /* DUNE_VFS_H */
