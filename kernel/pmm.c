#include <airlys/pmm.h>
#include <airlys/vga.h>

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

static uint8_t* pmm_bitmap = NULL;
static size_t pmm_total_frames = 0;
static size_t pmm_used_frames = 0;

static inline void pmm_set_bit(size_t frame) {
    pmm_bitmap[frame / 8] |= (1 << (frame % 8));
}

static inline void pmm_clear_bit(size_t frame) {
    pmm_bitmap[frame / 8] &= ~(1 << (frame % 8));
}

static inline bool pmm_test_bit(size_t frame) {
    return (pmm_bitmap[frame / 8] & (1 << (frame % 8))) != 0;
}

void pmm_init(uintptr_t mem_size) {
    if (mem_size > PMM_MAX_MEMORY || mem_size == 0) {
        mem_size = PMM_MAX_MEMORY;
    }

    pmm_total_frames = mem_size / PAGE_SIZE;
    pmm_used_frames = pmm_total_frames; /* Initially mark all as used */

    /* Place bitmap directly at _kernel_end (page aligned) */
    uintptr_t bitmap_addr = ((uintptr_t)_kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    pmm_bitmap = (uint8_t*)bitmap_addr;

    /* Fill bitmap with 1s (all used) */
    size_t bitmap_bytes = pmm_total_frames / 8;
    for (size_t i = 0; i < bitmap_bytes; i++) {
        pmm_bitmap[i] = 0xFF;
    }

    /* Usable memory starts immediately after the bitmap */
    uintptr_t free_mem_start = bitmap_addr + bitmap_bytes;
    free_mem_start = (free_mem_start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    size_t start_frame = free_mem_start / PAGE_SIZE;
    for (size_t frame = start_frame; frame < pmm_total_frames; frame++) {
        pmm_clear_bit(frame);
        pmm_used_frames--;
    }
}

void* pmm_alloc_frame(void) {
    size_t bitmap_bytes = pmm_total_frames / 8;

    for (size_t i = 0; i < bitmap_bytes; i++) {
        if (pmm_bitmap[i] != 0xFF) {
            for (int bit = 0; bit < 8; bit++) {
                if (!(pmm_bitmap[i] & (1 << bit))) {
                    size_t frame = i * 8 + bit;
                    if (frame < pmm_total_frames) {
                        pmm_set_bit(frame);
                        pmm_used_frames++;
                        return (void*)(frame * PAGE_SIZE);
                    }
                }
            }
        }
    }
    return NULL; /* Out of physical memory */
}

void pmm_free_frame(void* frame_addr) {
    uintptr_t addr = (uintptr_t)frame_addr;
    size_t frame = addr / PAGE_SIZE;

    if (frame < pmm_total_frames && pmm_test_bit(frame)) {
        pmm_clear_bit(frame);
        pmm_used_frames--;
    }
}

void* pmm_alloc_frames(size_t count) {
    if (count == 0) return NULL;
    if (count == 1) return pmm_alloc_frame();

    size_t free_run = 0;
    size_t start_frame = 0;

    for (size_t frame = 0; frame < pmm_total_frames; frame++) {
        if (!pmm_test_bit(frame)) {
            if (free_run == 0) {
                start_frame = frame;
            }
            free_run++;
            if (free_run == count) {
                for (size_t j = start_frame; j < start_frame + count; j++) {
                    pmm_set_bit(j);
                    pmm_used_frames++;
                }
                return (void*)(start_frame * PAGE_SIZE);
            }
        } else {
            free_run = 0;
        }
    }
    return NULL;
}

void pmm_free_frames(void* frame_addr, size_t count) {
    uintptr_t addr = (uintptr_t)frame_addr;
    size_t start_frame = addr / PAGE_SIZE;

    for (size_t i = 0; i < count; i++) {
        size_t frame = start_frame + i;
        if (frame < pmm_total_frames && pmm_test_bit(frame)) {
            pmm_clear_bit(frame);
            pmm_used_frames--;
        }
    }
}

size_t pmm_get_total_frames(void) {
    return pmm_total_frames;
}

size_t pmm_get_used_frames(void) {
    return pmm_used_frames;
}

size_t pmm_get_free_frames(void) {
    return pmm_total_frames - pmm_used_frames;
}

uint64_t pmm_get_free_memory_kb(void) {
    return (uint64_t)(pmm_get_free_frames() * PAGE_SIZE) / 1024;
}
