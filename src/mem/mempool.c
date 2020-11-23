#include "mempool.h"
#include "utilcommon.h"
#include "utilmem.h"

/* global common memory pool */
mem_pool_t *mem_comm_pool = NULL;

/* get memory area size */
static ulint mem_area_get_size(mem_area_t *area) {
    return (area->size_and_free & ~MEM_AREA_FREE);
}

/* set memory area size */
static void mem_area_set_size(mem_area_t *area, ulint size) {
    area->size_and_free = (area->size_and_free & MEM_AREA_FREE) | size;
}

/* check if memory area is free */
static bool mem_area_get_free(mem_area_t *area) {
    return (area->size_and_free & MEM_AREA_FREE);
}

/* set memory area free bit */
static void mem_area_set_free(mem_area_t *area, bool free) {
    area->size_and_free = (area->size_and_free & ~MEM_AREA_FREE) | free;
}

mem_pool_t *mem_pool_create(ulint size) {
    mem_pool_t *pool;
    mem_area_t *area;
    ulint i;
    ulint used;
    pool = ut_malloc(sizeof(mem_pool_t));
    pool->buf = ut_malloc_low(size, FALSE);
    pool->size = size;
    mutex_create(&(pool->mutex));
    mutex_set_level(&(pool->mutex), SYNC_MEM_POOL);
    for (i = 0; i < 64; i++) {
        LIST_INIT(pool->free_list[i]);
    }
    used = 0;
    while (size - used >= MEM_AREA_MIN_SIZE) {
        i = ut_2_log(size - used);
        if (ut_2_exp(i) > size - used) {
            i--;
        }
        area = (mem_area_t *)(pool->buf + used);
        mem_area_set_size(area, ut_2_exp(i));
        mem_area_set_free(area, TRUE);
        LIST_ADD_FIRST(free_list, pool->free_list[i], area);
        used = used + ut_2_exp(i);
    }
    pool->reserved = 0;
    return pool;
}

/* fill the specified free list */
static bool mem_pool_fill_free_list(ulint i, mem_pool_t *pool) {
    mem_area_t *area;
    mem_area_t *area2;
    bool ret;
    /* run out of memory */
    if (i >= 63) {
        return FALSE;
    }
    area = LIST_GET_FIRST(pool->free_list[i + 1]);
    if (area == NULL) {
        ret = mem_pool_fill_free_list(i + 1, pool);
        /* run out of memory */
        if (ret == FALSE) {
            return FALSE;
        }
        area = LIST_GET_FIRST(pool->free_list[i + 1]);
    }
    LIST_REMOVE(free_list, pool->free_list[i + 1], area);
    area2 = (mem_area_t *)(((byte *)area) + ut_2_exp(i));
    mem_area_set_size(area2, ut_2_exp(i));
    mem_area_set_free(area2, TRUE);
    LIST_ADD_FIRST(free_list, pool->free_list[i], area2);
    mem_area_set_size(area, ut_2_exp(i));
    LIST_ADD_FIRST(free_list, pool->free_list[i], area);
    return TRUE;
}

void *mem_area_alloc(ulint size, mem_pool_t *pool) {
    mem_area_t *area;
    ulint n;
    bool ret;
    n = ut_2_log(ut_max(size + MEM_AREA_EXTRA_SIZE, MEM_AREA_MIN_SIZE));
    mutex_enter(&(pool->mutex));
    area = LIST_GET_FIRST(pool->free_list[n]);
    if (area == NULL) {
        ret = mem_pool_fill_free_list(n, pool);
        if (!ret) {
            /* out of memory in memory pool, try to allocate from OS with malloc */
            mutex_exit(&(pool->mutex));
            return (ut_malloc(size));
        }
        area = LIST_GET_FIRST(pool->free_list[n]);
    } 
    mem_area_set_free(area, FALSE);
    LIST_REMOVE(free_list, pool->free_list[n], area);
    pool->reserved += mem_area_get_size(area);
    mutex_exit(&(pool->mutex));
    return ((void *)(MEM_AREA_EXTRA_SIZE + ((byte *)area)));
}

/* gets the buddy of an area if exists */
static mem_area_t *mem_area_get_buddy(mem_area_t *area, ulint size, mem_pool_t *pool) {
    mem_area_t *buddy;
    if (((((byte *)area) - pool->buf) % (2 * size)) == 0) {
        /* buddy is in a higher address */
        buddy = (mem_area_t *)(((byte *)area) + size);
        if ((((byte *)buddy) - pool->buf) + size > pool->size) {
            buddy = NULL;
        }
    } else {
        /* buddy is in a lower address */
        buddy = (mem_area_t *)(((byte *)area) - size);
    }
    return buddy;
}

void mem_area_free(void *p, mem_pool_t *pool) {

}


ulint mem_pool_get_reserved(mem_pool_t *pool) {
    ulint reserved;
    mutex_enter(&(pool->mutex));
    reserved = pool->reserved;
    mutex_exit(&(pool->mutex));
    return reserved;
}

void mem_pool_mutex_enter() {
    mutex_enter(&(mem_comm_pool->mutex));
}

void mem_pool_mutex_exit() {
    mutex_exit(&(mem_comm_pool->mutex));
}

