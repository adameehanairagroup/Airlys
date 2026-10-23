#ifndef AIRLYS_VIRTIO_H
#define AIRLYS_VIRTIO_H

/*
 * Airlys Operating System Kernel - Virtualization & VirtIO (Linux virt/ Parity)
 * Lead Architect: Er. Adam Eehan (Founder & CEO, Aira Group of Technology)
 * Co-Pilot: Sovereign AI Aira 3 Ultra (Engine v3.6)
 */

#include <airlys/types.h>

/* VirtIO Subsystem Device Types */
#define VIRTIO_DEV_NET      1
#define VIRTIO_DEV_BLOCK    2
#define VIRTIO_DEV_CONSOLE  3
#define VIRTIO_DEV_RNG      4
#define VIRTIO_DEV_BALLOON  5
#define VIRTIO_DEV_SCSI     8
#define VIRTIO_DEV_GPU      16

/* VirtIO Status Bits */
#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER      2
#define VIRTIO_STATUS_DRIVER_OK   4
#define VIRTIO_STATUS_FAILED      128

typedef struct {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed)) vring_desc_t;

typedef struct {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[16];
} __attribute__((packed)) vring_avail_t;

typedef struct {
    uint32_t id;
    uint32_t len;
} __attribute__((packed)) vring_used_elem_t;

typedef struct {
    uint16_t          flags;
    uint16_t          idx;
    vring_used_elem_t ring[16];
} __attribute__((packed)) vring_used_t;

typedef struct {
    uint16_t io_base;
    uint16_t device_id;
    uint8_t  status;
    bool     active;
} virtio_device_t;

#define MAX_VIRTIO_DEVS 8

/* VirtIO Subsystem API */
void virtio_init(void);
int virtio_register_dev(uint16_t io_base, uint16_t device_id);
uint32_t virtio_get_device_count(void);
virtio_device_t* virtio_get_device(uint32_t index);

#endif /* AIRLYS_VIRTIO_H */
