#ifndef AIRLYS_DEVFS_H
#define AIRLYS_DEVFS_H

#include <airlys/vfs.h>

/* Initialize /dev character devices (null, zero, urandom, console) */
void devfs_init(void);

#endif /* AIRLYS_DEVFS_H */
