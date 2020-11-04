#include "cache.h"
#ifdef CACHE_SYSTEM_MALLOC


/*
 * default memory allocator by simply wrapping system calls
 */
static void *mem_malloc(size_t size) {
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

static void *mem_free(void *p, size_t size) {
    int64_t *ptr = (int64_t *)p;
    assert(ptr != 0);
    ptr--;
    free(ptr);
}

static void *mem_realloc(void *p, size_t old_size, size_t new_size) {
    int64_t *ptr = (int64_t *)p;
    // assert(ptr != 0 && size > 0);
    new_size = ROUND8(new_size);
    ptr--;
    ptr = realloc(ptr, new_size + 8);
    if (ptr) {
        ptr[0] = new_size;
        ptr++;
    } else {
        // TODO log

    }
    return (void *)ptr;
}

/*
static size_t mem_size(void *p) {
    int64_t *ptr;
    if (p == 0) {
        return 0;
    }
    ptr = (int64_t *)p;
    ptr--;
    return (size_t)ptr[0];
}
*/

static size_t mem_roundup(size_t size) {
    return ROUND8(size);
}

static int mem_init(void *not_used) {
    return CACHE_OK;
}

static void mem_shutdown(void *not_used) {
    return;
}

// export the implementations 
void cache_mem_set_default(void) {
    printf("use mem1\n");
    static const cache_mem_methods default_methods = {
        mem_malloc,
        mem_free,
        mem_realloc,
        mem_roundup,
        mem_init,
        mem_shutdown
    };
    cache_config(CACHE_CONFIG_ALLOC, &default_methods);
}

#endif  // CACHE_SYSTEM_MALLOC 
