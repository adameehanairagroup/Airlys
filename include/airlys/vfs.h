#ifndef AIRLYS_VFS_H
#define AIRLYS_VFS_H

#include <airlys/types.h>

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_CHARDEVICE  0x03
#define VFS_BLOCKDEVICE 0x04

#define VFS_MAX_NAME    64
#define VFS_MAX_CHILDREN 32

struct vfs_node;

typedef uint64_t (*vfs_read_fn)(struct vfs_node* node, uint64_t offset, uint64_t size, uint8_t* buffer);
typedef uint64_t (*vfs_write_fn)(struct vfs_node* node, uint64_t offset, uint64_t size, const uint8_t* buffer);
typedef struct vfs_node* (*vfs_finddir_fn)(struct vfs_node* node, const char* name);
typedef struct vfs_node* (*vfs_readdir_fn)(struct vfs_node* node, uint32_t index);

typedef struct vfs_node {
    char name[VFS_MAX_NAME];
    uint32_t flags;
    uint64_t size;
    uint32_t inode;
    vfs_read_fn read;
    vfs_write_fn write;
    vfs_finddir_fn finddir;
    vfs_readdir_fn readdir;
    struct vfs_node* parent;
    struct vfs_node* children[VFS_MAX_CHILDREN];
    uint32_t num_children;
    void* internal_data;
} vfs_node_t;

/* Initialize Virtual File System */
void vfs_init(void);

/* Root VFS node */
vfs_node_t* vfs_get_root(void);

/* Set new root VFS node */
void vfs_set_root(vfs_node_t* root);

/* Path resolution helper (e.g. "/etc/os-release" or "os-release") */
vfs_node_t* vfs_resolve_path(const char* path);

/* Standard VFS operations */
uint64_t vfs_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer);
uint64_t vfs_write(vfs_node_t* node, uint64_t offset, uint64_t size, const uint8_t* buffer);
vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name);
vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index);

#endif /* AIRLYS_VFS_H */
