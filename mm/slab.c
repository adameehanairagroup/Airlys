#include <airlys/slab.h>
#include <airlys/heap.h>
#include <airlys/string.h>

#define MAX_SLAB_CACHES 16

static kmem_cache_t slab_caches[MAX_SLAB_CACHES];
static size_t num_caches = 0;

void slab_init(void) {
    num_caches = 0;
    for (size_t i = 0; i < MAX_SLAB_CACHES; i++) {
        slab_caches[i].obj_size = 0;
        slab_caches[i].free_list = NULL;
    }
}

kmem_cache_t* kmem_cache_create(const char* name, size_t size, size_t align) {
    if (num_caches >= MAX_SLAB_CACHES || size == 0) return NULL;

    kmem_cache_t* cache = &slab_caches[num_caches++];
    kstrncpy(cache->name, name ? name : "slab", sizeof(cache->name));

    /* Ensure size is at least pointer-sized for free list linkage */
    if (size < sizeof(void*)) size = sizeof(void*);

    /* Align size */
    size_t a = (align > 0) ? align : 8;
    cache->obj_size = (size + a - 1) & ~(a - 1);
    cache->align = a;
    cache->total_alloc = 0;
    cache->free_count = 0;
    cache->free_list = NULL;

    return cache;
}

void* kmem_cache_alloc(kmem_cache_t* cache) {
    if (!cache) return NULL;

    if (cache->free_list) {
        void* obj = cache->free_list;
        cache->free_list = *(void**)obj;
        cache->free_count--;
        return obj;
    }

    /* Allocate new chunk from kernel heap */
    void* obj = kmalloc(cache->obj_size);
    if (obj) {
        cache->total_alloc++;
    }
    return obj;
}

void kmem_cache_free(kmem_cache_t* cache, void* obj) {
    if (!cache || !obj) return;

    /* Push into free list */
    *(void**)obj = cache->free_list;
    cache->free_list = obj;
    cache->free_count++;
}
