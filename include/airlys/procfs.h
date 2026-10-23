#ifndef AIRLYS_PROCFS_H
#define AIRLYS_PROCFS_H

#include <airlys/vfs.h>

/* Mount Linux-style /proc pseudo-filesystem into VFS */
void procfs_init(void);

#endif /* AIRLYS_PROCFS_H */
