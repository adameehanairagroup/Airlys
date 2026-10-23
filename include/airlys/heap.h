#ifndef AIRLYS_HEAP_H
#define AIRLYS_HEAP_H

#include <airlys/types.h>

#define KERNEL_HEAP_START   0x01000000ULL /* 16 MB boundary */
#define KERNEL_HEAP_SIZE    (32ULL * 1024 * 1024) /* 32 MB heap pool */

/* Initialize dynamic kernel heap allocator */
void heap_init(uintptr_t start_addr, size_t size);

/* Allocate dynamic heap memory */
void* kmalloc(size_t size);

/* Free previously allocated heap memory */
void kfree(void* ptr);

/* Allocate zero-initialized dynamic heap memory */
void* kcalloc(size_t num, size_t size);

/* Reallocate dynamic heap memory with new size */
void* krealloc(void* ptr, size_t new_size);

/* Heap Statistics */
size_t heap_get_total_bytes(void);
size_t heap_get_used_bytes(void);
size_t heap_get_free_bytes(void);

#endif /* AIRLYS_HEAP_H */
