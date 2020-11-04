#include "cache.h"
#ifdef CACHE_ZERO_MALLOC

/*
 * no-op memory allocation methods
 * default implementations
 */
static void *mem_malloc(size_t size) {
    return 0;
}

static void mem_free(void *p, size_t size) {
    return;
}

static void mem_realloc(void *p, size_t old_size, size_t new_size) {
    return 0;
}

/*
static size_t mem_size(void *p) {
    return 0;
}
*/

static size_t mem_roundup(size_t size) {
    return size;
}

static int mem_init(void *not_used) {
    return CACHE_OK;
}

static int mem_shutdown(void *not_used) {
    return;
}

// export the implementations 
void cache_mem_set_default(void) {
    printf("use mem0\n");
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

#endif  // CACHE_ZERO_MALLOC 

