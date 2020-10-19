#include "cache.h"
#ifdef CACHE_SYSTEM_MALLOC


/*
 * default memory allocator by simply wrapping system calls
 * 
 */
static void *cache_mem_malloc(int size) {
    int64_t *p;
    assert(size > 0);
    size = ROUND8(size);
    p = malloc(size + 8);
    if (p) {
        p[0] = size;
        p++;
    } else {
        // log
    }
    return (void *)p;
}

// static void *cache_mem


#endif /* CACHE_SYSTEM_MALLOC */
