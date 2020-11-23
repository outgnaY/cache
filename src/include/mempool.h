#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "common.h"
#include "list.h"

/* extra bytes for control information */
#define MEM_AREA_EXTRA_SIZE (calc_align(sizeof(struct mem_area_struct), MEM_ALIGNMENT))

/* mask used to extract free bit from area->size */
#define MEM_AREA_FREE 1

/* the smallest memory area total size */
#define MEM_AREA_MIN_SIZE (2 * MEM_AREA_EXTRA_SIZE)

typedef struct mem_area_struct mem_area_t;
typedef struct mem_pool_struct mem_pool_t;

/* global common memory pool */
extern mem_pool_t *mem_comm_pool;

/* memory area header */
struct mem_area_struct {
    ulint size_and_free;
    LIST_NODE_T(mem_area_t) free_list;   /* free list node */
};

struct mem_pool_struct {
    byte *buf;                                      /* memory pool */
    ulint size;                                     /* memory common pool size */
    ulint reserved;                                 /* currently allocated memory */
    // mutex_t mutex;                                  /* mutex protecting this struct */
    LIST_BASE_NODE_T(mem_area_t) free_list[64];     /* list of free memory areas */
};

/* create a memory pool */
mem_pool_t *mem_pool_create(ulint size);

/* allocates memory from a pool */
void *mem_area_alloc(ulint size, mem_pool_t *pool);

/* free memory to a pool */
void mem_area_free(void *p, mem_pool_t *pool);

/* return the amount of reserved memory */
ulint mem_pool_get_reserved(mem_pool_t *pool);

void mem_pool_mutex_enter(void);

void mem_pool_mutex_exit(void);

#endif  /* MEM_POOL_H */