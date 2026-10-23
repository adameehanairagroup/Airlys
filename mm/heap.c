#include <airlys/heap.h>
#include <airlys/vga.h>
#include <airlys/kernel.h>

#define HEAP_MAGIC 0xA18A3644ULL

typedef struct heap_block {
    size_t size;              /* Payload size in bytes */
    bool is_free;             /* True if block is free */
    uint32_t magic;           /* Integrity guard */
    struct heap_block* next;  /* Next block in list */
    struct heap_block* prev;  /* Previous block */
} __attribute__((aligned(16))) heap_block_t;

static uintptr_t heap_start = 0;
static size_t heap_total_size = 0;
static heap_block_t* heap_head = NULL;

static inline size_t align16(size_t size) {
    return (size + 15) & ~15ULL;
}

void heap_init(uintptr_t start_addr, size_t size) {
    heap_start = (start_addr + 15) & ~15ULL;
    heap_total_size = size;

    heap_head = (heap_block_t*)heap_start;
    heap_head->size = size - sizeof(heap_block_t);
    heap_head->is_free = true;
    heap_head->magic = HEAP_MAGIC;
    heap_head->next = NULL;
    heap_head->prev = NULL;
}

void* kmalloc(size_t size) {
    if (size == 0 || heap_head == NULL) {
        return NULL;
    }

    size_t aligned_size = align16(size);
    heap_block_t* curr = heap_head;

    while (curr) {
        if (curr->magic != HEAP_MAGIC) {
            kernel_panic("Kernel Heap Corruption detected: invalid magic token!");
        }

        if (curr->is_free && curr->size >= aligned_size) {
            /* Check if block can be split */
            if (curr->size >= aligned_size + sizeof(heap_block_t) + 32) {
                uintptr_t new_block_addr = (uintptr_t)curr + sizeof(heap_block_t) + aligned_size;
                heap_block_t* new_block = (heap_block_t*)new_block_addr;

                new_block->size = curr->size - aligned_size - sizeof(heap_block_t);
                new_block->is_free = true;
                new_block->magic = HEAP_MAGIC;
                new_block->next = curr->next;
                new_block->prev = curr;

                if (curr->next) {
                    curr->next->prev = new_block;
                }
                curr->next = new_block;
                curr->size = aligned_size;
            }

            curr->is_free = false;
            return (void*)((uint8_t*)curr + sizeof(heap_block_t));
        }
        curr = curr->next;
    }

    return NULL; /* Out of heap memory */
}

void kfree(void* ptr) {
    if (!ptr) return;

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));

    if (block->magic != HEAP_MAGIC) {
        kernel_panic("Kernel kfree error: double free or invalid memory pointer!");
    }

    block->is_free = true;

    /* Coalesce with next block if free */
    if (block->next && block->next->is_free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    /* Coalesce with prev block if free */
    if (block->prev && block->prev->is_free) {
        block->prev->size += sizeof(heap_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

void* kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = kmalloc(total);
    if (ptr) {
        uint8_t* p = (uint8_t*)ptr;
        for (size_t i = 0; i < total; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}

void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        kernel_panic("Kernel krealloc error: invalid magic header!");
    }

    if (block->size >= new_size) {
        return ptr; /* Existing block is already large enough */
    }

    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) return NULL;

    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;
    for (size_t i = 0; i < block->size; i++) {
        dst[i] = src[i];
    }

    kfree(ptr);
    return new_ptr;
}

size_t heap_get_total_bytes(void) {
    return heap_total_size;
}

size_t heap_get_used_bytes(void) {
    size_t used = 0;
    heap_block_t* curr = heap_head;
    while (curr) {
        if (!curr->is_free) {
            used += curr->size + sizeof(heap_block_t);
        }
        curr = curr->next;
    }
    return used;
}

size_t heap_get_free_bytes(void) {
    size_t free_mem = 0;
    heap_block_t* curr = heap_head;
    while (curr) {
        if (curr->is_free) {
            free_mem += curr->size;
        }
        curr = curr->next;
    }
    return free_mem;
}
