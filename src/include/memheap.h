#ifndef MEM_H
#define MEM_H

#include "common.h"
#include "list.h"

/* memory heaps */
/* stored at the beginning of a heap block */
typedef struct mem_block_info_struct mem_block_info_t;

typedef mem_block_info_t mem_block_t;

/* a memory heap is a list of memory blocks */
typedef mem_block_t mem_heap_t;

/* 
 * start size is used for the first block in the heap if the size is not specified
 * standard size is the maximum size of the blocks used for allocations of small buffers
 */
#define MEM_BLOCK_START_SIZE    64
#define MEM_BLOCK_STANDARD_SIZE 8192    

/**
 * DYNAMIC: allocation from the dynamic memory pool
 * BUFFER: allocation from index page buffer pool
 */
#define MEM_HEAP_DYNAMIC 0
#define MEM_HEAP_BUFFER 1

mem_block_t *mem_heap_create_block(mem_heap_t *heap, ulint n, void *init_block, ulint type);

/* initialize the memory system */
void mem_init(ulint size);

#define mem_alloc(N) mem_alloc_func(N)
#define mem_free(P) mem_free_func(p)
#define mem_realloc(P, N) mem_realloc_func(P, N)

static void *mem_alloc_func(ulint n);

static void mem_free_func(void *p);

static void mem_realloc_func(void *p, ulint n);


/* header info of a block */
struct mem_block_info_struct {
    LIST_BASE_NODE_T(mem_block_t) base;     /* base node of the list */
    LIST_NODE_T(mem_block_t) list;          /* list */
    ulint len;                              /* length of the block */
    ulint type;                             /* type of heap */
    bool init_block;                        /* TRUE if this is first block */
    ulint free;                             /* first free position in the block */
    ulint start;                            /* initial free position */
};

/* header size of a memory heap block */
#define MEM_BLOCK_HEADER_SIZE   

#define MEM_SPACE_NEEDED(N) calc_align((N), MEM_ALIGNMENT)

#endif  /* MEM_H */