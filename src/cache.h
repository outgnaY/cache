
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

/* slab sizing definitions */
#define POWER_SMALLEST 1

/* max number of slab classes */
#define MAX_NUMBER_OF_SLAB_CLASSES (63 + 1)


/* global settings */
struct settings {
    int item_size_max;      /* maximum item size */
};

/**
 * stored item structure
 */
typedef struct item {
    struct item *next;
    struct item *prev;
    uint8_t slabs_clsid;

} item;

/*
 * result codes
 */
#define CACHE_OK 0                          /* successful result */

/*
 * round up a number to the next larger multiple of 8.
 * used to force 8-byte alignment on 64-bit architectures.
 */
#define ROUND8(X)   (((x) + 7) & ~7)



/*
 * memory allocation methods
 */
typedef struct cache_mem_methods cache_mem_methods;
struct cache_mem_methods {
    void *(*mem_malloc)(int);               /* memory allocation function */
    void *(*mem_free)(void *);              /* memory free function */
    void *(*mem_realloc)(void *, int);      /* memory resize function */
    int (*mem_size)(void *);                /* return the size of an allocation */
    int (*mem_roundup)(int);                /* round up the request size to allocation size */
    int (*mem_init)(void *);                /* initialize the memory allocator */
    int (*mem_shutdown)(void *);            /* finalize the memory allocator */
};