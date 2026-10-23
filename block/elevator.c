#include <airlys/block.h>
#include <airlys/printk.h>

typedef struct {
    uint64_t lba;
    uint32_t count;
    uint8_t* buffer;
    bool     is_write;
} blk_request_t;

#define ELEVATOR_QUEUE_SIZE 16

static blk_request_t s_elevator_queue[ELEVATOR_QUEUE_SIZE];
static uint32_t s_head = 0;
static uint32_t s_tail = 0;
static uint32_t s_count = 0;

void elevator_init(void) {
    s_head = s_tail = s_count = 0;
    printk(KERN_INFO "io-sched: No-Op / FIFO elevator scheduler initialized\n");
}

int elevator_add_request(uint64_t lba, uint32_t count, uint8_t* buffer, bool is_write) {
    if (s_count >= ELEVATOR_QUEUE_SIZE) {
        return -1; /* Queue full */
    }
    s_elevator_queue[s_tail].lba = lba;
    s_elevator_queue[s_tail].count = count;
    s_elevator_queue[s_tail].buffer = buffer;
    s_elevator_queue[s_tail].is_write = is_write;
    s_tail = (s_tail + 1) % ELEVATOR_QUEUE_SIZE;
    s_count++;
    return 0;
}

int elevator_dispatch(block_device_t* dev) {
    if (s_count == 0 || !dev) return 0;

    blk_request_t* req = &s_elevator_queue[s_head];
    int res = 0;
    if (req->is_write) {
        res = blk_write(dev, req->lba, req->count, req->buffer);
    } else {
        res = blk_read(dev, req->lba, req->count, req->buffer);
    }

    s_head = (s_head + 1) % ELEVATOR_QUEUE_SIZE;
    s_count--;
    return res;
}
