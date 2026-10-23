#include <airlys/devfs.h>
#include <airlys/ramfs.h>
#include <airlys/vga.h>
#include <airlys/timer.h>

static uint64_t dev_null_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    (void)node; (void)offset; (void)size; (void)buffer;
    return 0; /* EOF */
}

static uint64_t dev_null_write(vfs_node_t* node, uint64_t offset, uint64_t size, const uint8_t* buffer) {
    (void)node; (void)offset; (void)buffer;
    return size; /* Discarded */
}

static uint64_t dev_zero_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    (void)node; (void)offset;
    for (uint64_t i = 0; i < size; i++) {
        buffer[i] = 0;
    }
    return size;
}

static uint64_t dev_urandom_read(vfs_node_t* node, uint64_t offset, uint64_t size, uint8_t* buffer) {
    (void)node; (void)offset;
    static uint32_t seed = 0xA18A3644;
    seed ^= (uint32_t)timer_get_ticks();

    for (uint64_t i = 0; i < size; i++) {
        seed = seed * 1664525U + 1013904223U;
        buffer[i] = (uint8_t)(seed >> 16);
    }
    return size;
}

static uint64_t dev_console_write(vfs_node_t* node, uint64_t offset, uint64_t size, const uint8_t* buffer) {
    (void)node; (void)offset;
    for (uint64_t i = 0; i < size; i++) {
        vga_putchar((char)buffer[i]);
    }
    return size;
}

void devfs_init(void) {
    vfs_node_t* root = vfs_get_root();
    if (!root) return;

    vfs_node_t* dev = vfs_finddir(root, "dev");
    if (!dev) {
        dev = ramfs_create_dir(root, "dev");
    }
    if (!dev) return;

    /* /dev/null */
    vfs_node_t* null_node = ramfs_create_file(dev, "null", NULL);
    if (null_node) {
        null_node->flags = VFS_CHARDEVICE;
        null_node->read = dev_null_read;
        null_node->write = dev_null_write;
    }

    /* /dev/zero */
    vfs_node_t* zero_node = ramfs_create_file(dev, "zero", NULL);
    if (zero_node) {
        zero_node->flags = VFS_CHARDEVICE;
        zero_node->read = dev_zero_read;
        zero_node->write = dev_null_write;
    }

    /* /dev/urandom */
    vfs_node_t* rnd_node = ramfs_create_file(dev, "urandom", NULL);
    if (rnd_node) {
        rnd_node->flags = VFS_CHARDEVICE;
        rnd_node->read = dev_urandom_read;
        rnd_node->write = dev_null_write;
    }

    /* /dev/console */
    vfs_node_t* con_node = ramfs_create_file(dev, "console", NULL);
    if (con_node) {
        con_node->flags = VFS_CHARDEVICE;
        con_node->read = dev_null_read;
        con_node->write = dev_console_write;
    }
}
