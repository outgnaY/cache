#include "cache.h"
#ifdef CACHE_ZERO_MALLOC

/*
 * no-op memory allocation methods
 * default implementations
 */
static void *cache_mem_malloc(int size) {
    return 0;
}

static void cache_mem_free(void *p) {
    return;
}

static void cache_mem_realloc(void *p, int size) {
    return 0;
}

static int cache_mem_size(void *p) {
    return 0;
}

static int cache_mem_roundup(int size) {
    return size;
}

static int cache_mem_init(void *not_used) {
    return CACHE_OK;
}

static int cache_mem_shutdown(void *not_used) {
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

#endif /* CACHE_ZERO_MALLOC */

