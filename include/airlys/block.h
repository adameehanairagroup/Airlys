#ifndef AIRLYS_BLOCK_H
#define AIRLYS_BLOCK_H

/*
 * Airlys Operating System Kernel - Block Layer Subsystem (Linux block/ Parity)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

#define SECTOR_SIZE       512
#define MAX_BLOCK_DEVICES 8
#define MAX_PARTITIONS    16

/* Partition Types */
#define PART_TYPE_EMPTY   0x00
#define PART_TYPE_FAT32   0x0B
#define PART_TYPE_LINUX   0x83
#define PART_TYPE_SWAP    0x82
#define PART_TYPE_AIRLYS  0xEE

typedef struct {
    uint8_t  status;        /* 0x80 = bootable, 0x00 = non-bootable */
    uint8_t  first_chs[3];
    uint8_t  type;          /* Partition type */
    uint8_t  last_chs[3];
    uint32_t first_lba;     /* Logical Block Address */
    uint32_t sector_count;  /* Total sectors */
} __attribute__((packed)) mbr_partition_entry_t;

typedef struct {
    uint8_t              part_id;
    uint8_t              type;
    bool                 bootable;
    uint64_t             lba_start;
    uint64_t             total_sectors;
    char                 label[32];
} partition_info_t;

struct block_device;

typedef int (*blk_read_fn)(struct block_device* dev, uint64_t lba, uint32_t count, uint8_t* buffer);
typedef int (*blk_write_fn)(struct block_device* dev, uint64_t lba, uint32_t count, const uint8_t* buffer);

typedef struct block_device {
    char             name[32];      /* e.g., "sda", "vda", "ramdisk0" */
    uint32_t         dev_id;
    uint64_t         total_sectors;
    uint32_t         sector_size;
    bool             read_only;
    blk_read_fn      read;
    blk_write_fn     write;
    partition_info_t partitions[MAX_PARTITIONS];
    uint32_t         num_partitions;
} block_device_t;

/* Block Subsystem API */
void blk_init(void);
int blk_register_device(block_device_t* dev);
block_device_t* blk_get_device(const char* name);
uint32_t blk_get_device_count(void);
block_device_t* blk_get_device_by_index(uint32_t index);
int blk_read(block_device_t* dev, uint64_t lba, uint32_t count, uint8_t* buffer);
int blk_write(block_device_t* dev, uint64_t lba, uint32_t count, const uint8_t* buffer);

/* Partition Scanner */
int blk_scan_partitions(block_device_t* dev, const uint8_t* mbr_sector);

#endif /* AIRLYS_BLOCK_H */
