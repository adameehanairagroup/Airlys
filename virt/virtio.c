#include <airlys/virtio.h>
#include <airlys/string.h>
#include <airlys/printk.h>

static virtio_device_t g_virtio_devices[MAX_VIRTIO_DEVS];
static uint32_t        g_num_virtio_devices = 0;

void virtio_init(void) {
    g_num_virtio_devices = 0;
    kmemset(g_virtio_devices, 0, sizeof(g_virtio_devices));

    /* Register paravirtualized QEMU standard devices */
    virtio_register_dev(0xC000, VIRTIO_DEV_NET);
    virtio_register_dev(0xC040, VIRTIO_DEV_BLOCK);
    virtio_register_dev(0xC080, VIRTIO_DEV_CONSOLE);

    printk(KERN_INFO "virt: VirtIO Paravirtualization layer online (%u virtual devices ready)\n",
           g_num_virtio_devices);
}

int virtio_register_dev(uint16_t io_base, uint16_t device_id) {
    if (g_num_virtio_devices >= MAX_VIRTIO_DEVS) return -1;

    virtio_device_t* dev = &g_virtio_devices[g_num_virtio_devices];
    dev->io_base = io_base;
    dev->device_id = device_id;
    dev->status = VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_DRIVER_OK;
    dev->active = true;

    g_num_virtio_devices++;
    return 0;
}

uint32_t virtio_get_device_count(void) {
    return g_num_virtio_devices;
}

virtio_device_t* virtio_get_device(uint32_t index) {
    if (index >= g_num_virtio_devices) return NULL;
    return &g_virtio_devices[index];
}
