#include <airlys/block.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static block_device_t g_block_devices[MAX_BLOCK_DEVICES];
static uint32_t       g_num_block_devices = 0;

static uint8_t s_ramdisk_storage[1024 * 512]; /* 512 KB kernel ramdisk */

static int ramdisk_read(block_device_t* dev, uint64_t lba, uint32_t count, uint8_t* buffer) {
    (void)dev;
    uint64_t offset = lba * SECTOR_SIZE;
    uint64_t bytes = (uint64_t)count * SECTOR_SIZE;
    if (offset + bytes > sizeof(s_ramdisk_storage)) {
        return -1;
    }
    kmemcpy(buffer, s_ramdisk_storage + offset, bytes);
    return 0;
}

static int ramdisk_write(block_device_t* dev, uint64_t lba, uint32_t count, const uint8_t* buffer) {
    (void)dev;
    uint64_t offset = lba * SECTOR_SIZE;
    uint64_t bytes = (uint64_t)count * SECTOR_SIZE;
    if (offset + bytes > sizeof(s_ramdisk_storage)) {
        return -1;
    }
    kmemcpy(s_ramdisk_storage + offset, buffer, bytes);
    return 0;
}

static block_device_t s_ramdisk_dev = {
    .name = "ram0",
    .dev_id = 0,
    .total_sectors = (sizeof(s_ramdisk_storage) / SECTOR_SIZE),
    .sector_size = SECTOR_SIZE,
    .read_only = false,
    .read = ramdisk_read,
    .write = ramdisk_write,
    .num_partitions = 0
};

void blk_init(void) {
    g_num_block_devices = 0;
    kmemset(g_block_devices, 0, sizeof(g_block_devices));
    kmemset(s_ramdisk_storage, 0, sizeof(s_ramdisk_storage));

    /* Register default ramdisk block device */
    blk_register_device(&s_ramdisk_dev);

    /* Setup synthetic MBR partition for test */
    s_ramdisk_dev.partitions[0].part_id = 1;
    s_ramdisk_dev.partitions[0].type = PART_TYPE_AIRLYS;
    s_ramdisk_dev.partitions[0].bootable = true;
    s_ramdisk_dev.partitions[0].lba_start = 2048;
    s_ramdisk_dev.partitions[0].total_sectors = 1000;
    kstrcpy(s_ramdisk_dev.partitions[0].label, "Airlys-Root");
    s_ramdisk_dev.num_partitions = 1;

    printk(KERN_INFO "block: Block layer initialized (%u device registered)\n", g_num_block_devices);
}

int blk_register_device(block_device_t* dev) {
    if (!dev || g_num_block_devices >= MAX_BLOCK_DEVICES) {
        return -1;
    }
    kmemcpy(&g_block_devices[g_num_block_devices], dev, sizeof(block_device_t));
    g_num_block_devices++;
    return 0;
}

block_device_t* blk_get_device(const char* name) {
    if (!name) return NULL;
    for (uint32_t i = 0; i < g_num_block_devices; i++) {
        if (kstrcmp(g_block_devices[i].name, name) == 0) {
            return &g_block_devices[i];
        }
    }
    return NULL;
}

uint32_t blk_get_device_count(void) {
    return g_num_block_devices;
}

block_device_t* blk_get_device_by_index(uint32_t index) {
    if (index >= g_num_block_devices) return NULL;
    return &g_block_devices[index];
}

int blk_read(block_device_t* dev, uint64_t lba, uint32_t count, uint8_t* buffer) {
    if (!dev || !dev->read || !buffer) return -1;
    return dev->read(dev, lba, count, buffer);
}

int blk_write(block_device_t* dev, uint64_t lba, uint32_t count, const uint8_t* buffer) {
    if (!dev || !dev->write || !buffer || dev->read_only) return -1;
    return dev->write(dev, lba, count, buffer);
}
