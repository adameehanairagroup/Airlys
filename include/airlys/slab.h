#ifndef AIRLYS_SLAB_H
#define AIRLYS_SLAB_H

#include <airlys/types.h>

#define SLAB_NAME_MAX 32

typedef struct kmem_cache {
    char name[SLAB_NAME_MAX];
    size_t obj_size;
    size_t align;
    size_t total_alloc;
    size_t free_count;
    void* free_list;
} kmem_cache_t;

/* Initialize Slab subsystem */
void slab_init(void);

/* Create named object cache (e.g. "task_cache", "dentry_cache") */
kmem_cache_t* kmem_cache_create(const char* name, size_t size, size_t align);

/* Allocate an object from the cache */
void* kmem_cache_alloc(kmem_cache_t* cache);

/* Free an object back to the cache */
void kmem_cache_free(kmem_cache_t* cache, void* obj);

#endif /* AIRLYS_SLAB_H */
