#include "cache.h"
#ifdef CACHE_SYSTEM_MALLOC


/*
 * default memory allocator by simply wrapping system calls
 * 
 */
static void *cache_mem_malloc(int size) {
    int64_t *ptr;
    assert(size > 0);
    size = ROUND8(size);
    ptr = malloc(size + 8);
    if (ptr) {
        ptr[0] = size;
        ptr++;
    } else {
        // TODO log
    }
    return (void *)ptr;
}

static void *cache_mem_free(void *p) {
    int64_t *ptr = (int64_t *)p;
    assert(ptr != 0);
    ptr--;
    free(ptr);
}

static void *cache_mem_realloc(void *p, int size) {
    int64_t *ptr = (int64_t *)p;
    assert(ptr != 0 && size > 0);
    size = ROUND8(size);
    ptr--;
    ptr = realloc(ptr, size + 8);
    if (ptr) {
        ptr[0] = size;
        ptr++;
    } else {
        // TODO log

    }
    return (void *)ptr
}

static int cache_mem_size(void *p) {
    int64_t *ptr;
    if (p == 0) {
        return 0;
    }
    ptr = (int64_t *)p;
    ptr--;
    return (int)ptr[0];
}

static int cache_mem_roundup(int size) {
    return ROUND8(size);
}

static int cache_mem_init(void *not_used) {
    return CACHE_OK;
}

static void cache_mem_shutdown(void *not_used) {
    return;
}

/* export the implementations */
void cache_mem_set_default(void) {
    static const cache_mem_methods default_methods = {
        cache_mem_malloc,
        cache_mem_free,
        cache_mem_realloc,
        cache_mem_size,
        cache_mem_roundup,
        cache_mem_init,
        cache_mem_shutdown
    };
    cache_config();
}

#endif /* CACHE_SYSTEM_MALLOC */
