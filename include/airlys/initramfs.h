#ifndef AIRLYS_INITRAMFS_H
#define AIRLYS_INITRAMFS_H

/*
 * Airlys Operating System Kernel - Initramfs Subsystem (Linux usr/ Parity)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define CPIO_NEWC_MAGIC "070701"
#define CPIO_TRAILER    "TRAILER!!!"

typedef struct {
    char c_magic[6];     /* "070701" */
    char c_ino[8];       /* Inode number */
    char c_mode[8];      /* File mode & permissions */
    char c_uid[8];       /* User ID */
    char c_gid[8];       /* Group ID */
    char c_nlink[8];     /* Number of hard links */
    char c_mtime[8];     /* Modification time */
    char c_filesize[8];  /* File size in bytes */
    char c_devmajor[8];  /* Major device */
    char c_devminor[8];  /* Minor device */
    char c_rdevmajor[8]; /* Major special device */
    char c_rdevminor[8]; /* Minor special device */
    char c_namesize[8];  /* Filename size (including null) */
    char c_check[8];     /* Checksum */
} __attribute__((packed)) cpio_header_t;

typedef struct {
    uint32_t total_files;
    uint32_t total_dirs;
    uint64_t total_bytes;
    bool     initialized;
} initramfs_status_t;

/* Initramfs API */
int initramfs_unpack(const void* data, size_t size);
void initramfs_init(void);
initramfs_status_t initramfs_get_status(void);

#endif /* AIRLYS_INITRAMFS_H */
