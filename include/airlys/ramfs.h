#ifndef AIRLYS_RAMFS_H
#define AIRLYS_RAMFS_H

#include <airlys/vfs.h>

/* Initialize in-memory RAM filesystem and standard directories */
vfs_node_t* ramfs_init(void);

/* Create directory under a parent directory */
vfs_node_t* ramfs_create_dir(vfs_node_t* parent, const char* name);

/* Create a file with given text content under a parent directory */
vfs_node_t* ramfs_create_file(vfs_node_t* parent, const char* name, const char* content);

/* Delete file or empty directory node */
bool ramfs_delete_node(vfs_node_t* parent, const char* name);

#endif /* AIRLYS_RAMFS_H */
