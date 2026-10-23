#ifndef AIRLYS_PMM_H
#define AIRLYS_PMM_H

#include <airlys/types.h>

#define PAGE_SIZE           4096
#define PMM_MAX_MEMORY      (1024ULL * 1024 * 1024) /* 1 GB */
#define PMM_MAX_FRAMES      (PMM_MAX_MEMORY / PAGE_SIZE) /* 262,144 frames */
#define PMM_BITMAP_SIZE     (PMM_MAX_FRAMES / 8)         /* 32,768 bytes */

/* Initialize Physical Memory Manager above kernel image */
void pmm_init(uintptr_t mem_size);

/* Allocate a single 4KB physical frame (returns physical address or 0 on failure) */
void* pmm_alloc_frame(void);

/* Free an allocated 4KB physical frame */
void pmm_free_frame(void* frame);

/* Allocate multiple contiguous physical frames */
void* pmm_alloc_frames(size_t count);

/* Free multiple contiguous physical frames */
void pmm_free_frames(void* frame, size_t count);

/* Statistics */
size_t pmm_get_total_frames(void);
size_t pmm_get_used_frames(void);
size_t pmm_get_free_frames(void);
uint64_t pmm_get_free_memory_kb(void);

#endif /* AIRLYS_PMM_H */
