#include "memheap.h"


/* setters and getters */
static void mem_block_set_len(mem_block_t *block, ulint len) {
    block->len = len;
}

static ulint mem_block_get_len(mem_block_t *block) {
    return block->len;
}

static void mem_block_set_type(mem_block_t *block, ulint type) {
    block->type = type;
}

static ulint mem_block_get_type(mem_block_t *block) {
    return block->type;
}

static void mem_block_set_free(mem_block_t *block, ulint free) {
    block->free = free;
}

static ulint mem_block_get_free(mem_block_t *block) {
    return block->free;
}

static void mem_block_set_start(mem_block_t *block, ulint start) {
    block->start = start;
}

static ulint mem_block_get_start(mem_block_t *block) {
    return block->start;
}

/* allocates n bytes of memory from a memory heap */
static void *mem_heap_alloc(mem_heap_t *heap, ulint n) {
    mem_block_t *block;
    void *buf;
    ulint free;
    block = LIST_GET_LAST(heap->base);
}

/**
 * create a memory heap block 
 * @param heap memory heap or NULL if first block should be created
 * @param n number of bytes need for user data, or if init_block is not NULL, its size in bytes
 * @param init_block init block, type must be MEM_HEAP_DYNAMIC
 * @param type heap type
 */
mem_block_t *mem_heap_create_block(mem_heap_t *heap, ulint n, void *init_block, ulint type) {
    mem_block_t *block;
    ulint len;
    /* calculate the size */
    if (init_block != NULL) {
        len = n;
        block = init_block;
    } else if (type == MEM_HEAP_DYNAMIC) {
        len = MEM_BLOCK_HEADER_SIZE + MEM_SPACE_NEEDED(n);
        block = mem_area_alloc(len, mem_comm_pool);
    } else {

    }
    if (block == NULL) {
        return NULL;
    }
    mem_block_set_len(block, len);
    mem_block_set_type(block, type);
    mem_block_set_free(block, MEM_BLOCK_HEADER_SIZE);
    mem_block_set_start(block, MEM_BLOCK_HEADER_SIZE);

    if (init_block != NULL) {
        block->init_block = TRUE;
    } else {
        block->init_block = FALSE;
    }
    return block;
}

/* add a new block to a memory heap */
mem_block_t *mem_heap_add_block(mem_heap_t *heap, ulint n) {
    mem_block_t *block;
    mem_block_t *new_block;
    ulint new_size;
    block = LIST_GET_LAST(heap->base);
    new_size = 2 * mem_block_get_len(block);
    if (heap->type != MEM_HEAP_DYNAMIC) {

    } else if (new_size > MEM_BLOCK_STANDARD_SIZE) {
        /* if type is MEM_HEAP_DYNAMIC, we can allocate MEM_BLOCK_STANDARD_SIZE at most */
        new_size = MEM_BLOCK_STANDARD_SIZE;
    }
    if (new_size < n) {
        new_size = n;
    }
    new_block = mem_heap_create_block(heap, new_size, NULL, heap->type);
    if (new_block == NULL) {
        return NULL;
    }
    LIST_INSERT_AFTER(list, heap->base, block, new_block);
    return new_block;
}

/* frees a block from a memory heap */
void mem_heap_block_free(mem_heap_t *heap, mem_block_t *block) {
    ulint len;
    
    LIST_REMOVE(list, heap->base, block);
    if (block->init_block) {
        /* do nothing */
    } else if (block->type == MEM_HEAP_DYNAMIC) {
        mem_area_free(block, mem_comm_pool);
    } else {

    }
}

static void *mem_alloc_func(ulint n) {

}

static void mem_free_func(void *p) {
    
}