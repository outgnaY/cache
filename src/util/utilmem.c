#include "utilmem.h"
#include "list.h"
#include "ossync.h"


ulint calc_align(ulint n, ulint align_no) {
    return ((n + align_no - 1) & ~(align_no - 1));
}

/* header */
typedef struct ut_mem_block_struct ut_mem_block_t;

/* total amount of memory allocated from OS with malloc */
ulint ut_total_allocated_memory = 0;

struct ut_mem_block_struct {
    LIST_NODE_T(ut_mem_block_t) mem_block_list;
    ulint size;     /* size of allocated memory */
};

/* list of all memory blocks allocated from OS with malloc */
LIST_BASE_NODE_T(ut_mem_block_t) ut_mem_block_list;

/* protec the list */
os_fast_mutex_t ut_list_mutex;   

bool ut_mem_block_list_inited = FALSE;

/* initializes the mem block list */
static void ut_mem_block_list_init(void) {
    os_fast_mutex_init(&ut_list_mutex);
    LIST_INIT(ut_mem_block_list);
    ut_mem_block_list_inited = TRUE;
}


void *ut_malloc_low(ulint n, bool set_to_zero) {
    void *ret;
    if (!ut_mem_block_list_inited) {
        ut_mem_block_list_init();
    }
    os_fast_mutex_lock(&ut_list_mutex);
    ret = malloc(n + sizeof(ut_mem_block_t));
    if (ret == NULL) {
        fprintf(stderr, "cannot allocate memory.\n");
        exit(1);
    }
    if (set_to_zero) {
        memset(ret, 0, n + sizeof(ut_mem_block_t));
    }
    ((ut_mem_block_t *)ret)->size = n + sizeof(ut_mem_block_t);
    ut_total_allocated_memory += n + sizeof(ut_mem_block_t);
    LIST_ADD_FIRST(mem_block_list, ut_mem_block_list, ((ut_mem_block_t *)ret));
    os_fast_mutex_unlock(&ut_list_mutex);
    return ((void *)((byte *)ret + sizeof(ut_mem_block_t)));
}

void *ut_malloc(ulint n) {
    return ut_malloc_low(n, TRUE);
}

void ut_free(void *p) {
    ut_mem_block_t *block;
    block = (ut_mem_block_t *)((byte *)p - sizeof(ut_mem_block_t));
    os_fast_mutex_lock(&ut_list_mutex);
    ut_total_allocated_memory -= block->size;
    LIST_REMOVE(mem_block_list, ut_mem_block_list, block);
    free(block);
    os_fast_mutex_unlock(&ut_list_mutex);
}

void ut_free_all_mem(void) {
    ut_mem_block_t *block;
    os_fast_mutex_lock(&ut_list_mutex);
    while ((block = LIST_GET_FIRST(ut_mem_block_list))) {
        ut_total_allocated_memory -= block->size;
        LIST_REMOVE(mem_block_list, ut_mem_block_list, block);
        free(block);
    }
    os_fast_mutex_unlock(&ut_list_mutex);
    assert(ut_total_allocated_memory == 0);
}


