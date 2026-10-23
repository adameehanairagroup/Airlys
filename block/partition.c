#include <airlys/block.h>
#include <airlys/string.h>
#include <airlys/printk.h>

int blk_scan_partitions(block_device_t* dev, const uint8_t* mbr_sector) {
    if (!dev || !mbr_sector) return -1;

    /* Verify MBR Boot Signature 0x55, 0xAA */
    if (mbr_sector[510] != 0x55 || mbr_sector[511] != 0xAA) {
        printk(KERN_WARNING "block: Device %s has no valid MBR signature\n", dev->name);
        return -2;
    }

    const mbr_partition_entry_t* entries = (const mbr_partition_entry_t*)(mbr_sector + 446);
    dev->num_partitions = 0;

    for (int i = 0; i < 4; i++) {
        const mbr_partition_entry_t* p = &entries[i];
        if (p->type == PART_TYPE_EMPTY || p->sector_count == 0) {
            continue;
        }

        partition_info_t* part = &dev->partitions[dev->num_partitions];
        part->part_id = dev->num_partitions + 1;
        part->type = p->type;
        part->bootable = (p->status == 0x80);
        part->lba_start = p->first_lba;
        part->total_sectors = p->sector_count;

        if (p->type == PART_TYPE_LINUX) {
            kstrcpy(part->label, "Linux-ext4");
        } else if (p->type == PART_TYPE_FAT32) {
            kstrcpy(part->label, "EFI-System");
        } else if (p->type == PART_TYPE_SWAP) {
            kstrcpy(part->label, "Linux-swap");
        } else if (p->type == PART_TYPE_AIRLYS) {
            kstrcpy(part->label, "Airlys-Sovereign");
        } else {
            kstrcpy(part->label, "Data-Partition");
        }

        dev->num_partitions++;
    }

    printk(KERN_INFO "block: Device %s scanned - found %u partitions\n", dev->name, dev->num_partitions);
    return (int)dev->num_partitions;
}
