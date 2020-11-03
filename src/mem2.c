#include "cache.h"
// #ifdef CACHE_ENABLE_MEMSYS2

/**
 * a simple two-level memory allocator implementation
 */
#define ALIGN 8
#define MAX_BYTES 4096
#define FREELIST_SIZE 512

/* a block in the freelist */
typedef union block {
    union block* next;
    char data[1];
} block;

/* mempool start */
static char *start_free;
/* mempool end */
static char *end_free;

static size_t heap_size;


static block* s_freelist[FREELIST_SIZE];
static pthread_mutex_t mem2_lock; 

/* index of freelist according to given size. start from 0 */
static size_t freelist_index(size_t size) {
    return (size + (size_t)ALIGN - 1) / (size_t)ALIGN - 1;
}

/* allocate chunk, assume that size is already properly aligned */
static void *chunk_alloc(size_t size, int *pn) {
    char *result;
    /* total bytes */
    size_t total_bytes = size * (*pn);
    size_t bytes_left = end_free - start_free;
    block **my_freelist = 0;
    if (bytes_left >= total_bytes) {
        /* enough space */
        result = start_free;
        start_free += total_bytes;
        return result;
    } else if (bytes_left >= size) {
        /* provide at least one block */
        *pn = (int)(bytes_left / size);
        total_bytes = size * (*pn);
        result = start_free;
        start_free += total_bytes;
        return result;
    } else {
        /* cannot provide even one block */
        size_t bytes_to_get = 2 * total_bytes;
        /* try to make use of the left-over space of the mempool */
        if (bytes_left > 0) {
            my_freelist = s_freelist + freelist_index(bytes_left);
            ((block *)start_free)->next = *my_freelist;
            *my_freelist = (block *)start_free;
        }
        start_free = (char *)malloc(bytes_to_get);
        if (start_free == 0) {
            /* malloc failed, not enough heap space */
            size_t i;
            block *p;
            for (i = size; i <= MAX_BYTES; i += ALIGN) {
                my_freelist = s_freelist + freelist_index(i);
                p = *my_freelist;
                if (p != 0) {
                    /* rob a block and retry */
                    *my_freelist = p->next;
                    start_free = (char *)p;
                    end_free = start_free + i;
                    return chunk_alloc(size, pn);
                }
            }
            end_free = 0;
            start_free = (char *)malloc(bytes_to_get);

        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return chunk_alloc(size, pn);
    }
}

/* refill the mempool, assume that size is already properly aligned */
static void *refill(size_t size) {
    int n = 20;
    int *pn = &n;
    char *chunk = (char *)chunk_alloc(size, pn);
    block **my_freelist;
    block *result;
    block *current_block;
    block *next_block;
    int i;
    if (*pn == 1) {
        return chunk;
    }
    my_freelist = s_freelist + freelist_index(size);
    /* build freelist in chunk */
    result = (block *)chunk;
    *my_freelist = next_block = (block *)(chunk + size);
    for (i = 1; ; i++) {
        current_block = next_block;
        next_block = (block *)((char *)next_block + size);
        if (*pn - 1 == i) {
            current_block->next = 0;
            break;
        } else {
            current_block->next = next_block;
        }
    }

    return result;
}

static void *mem_alloc(size_t size) {
    void *ret = 0;
    block **my_freelist = 0;
    block *result = 0;
    /* if size is larger than max block in the mempool, use malloc directly */
    if (size > (size_t)MAX_BYTES) {
        ret = malloc(size);
    } else {
        /* find corresponding freelist head */
        my_freelist = s_freelist + freelist_index(size);
        /* lock */
        pthread_mutex_lock(&mem2_lock);
        result = *my_freelist;
        if (result == 0) {
            ret = refill(ROUNDUP(size, ALIGN));
        } else {
            *my_freelist = result->next;
            ret = result;
        }
        /* unlock */
        pthread_mutex_unlock(&mem2_lock);
    }

    return ret;
}

static void mem_free(void *p, size_t size) {
    block **my_freelist = 0;
    block *q = (block *)p;
    /* if size is larger than max block in the mempool, use free directly */
    if (size > (size_t)MAX_BYTES) {
        free(p);
    } else {
        /* return to freelist */
        my_freelist = s_freelist + freelist_index(size);
        /* lock */
        pthread_mutex_lock(&mem2_lock);
        q->next = *my_freelist;
        *my_freelist = q;
        /* unlock */
        pthread_mutex_unlock(&mem2_lock);
    }
}

static void *mem_realloc(void *p, size_t old_size, size_t new_size) {
    void *result = 0;
    size_t copy_size = 0;
    if (old_size > (size_t)MAX_BYTES && new_size > (size_t)MAX_BYTES) {
        return realloc(p, new_size);
    }
    /* don't need to reallocate */
    if (ROUNDUP(old_size, ALIGN) == ROUNDUP(new_size, ALIGN)) {
        return p;
    }
    result = mem_alloc(new_size);
    copy_size = new_size > old_size ? old_size : new_size;
    memcpy(result, p, copy_size);
    mem_free(p, old_size);
    return result;
}

static int mem_init() {
    int result = 0;
    start_free = 0;
    end_free = 0;
    heap_size = 0;
    int i;
    for (i = 0; i < FREELIST_SIZE; i++) {
        s_freelist[i] = 0;
    }
    if ((result = pthread_mutex_init(&mem2_lock, NULL)) != 0) {
        fprintf(stderr, "mempool mutex init failed\n");
    }
    return result;
}


typedef struct big {
    char arr[2048];
    int num;
} big_t;

big_t *pbig[100000];

int main() {
    /* mem alloc test */
    mem_init();
    
    int i = 0;
    // char *pch;
    // int *pint;
    printf("start memory allocate\n");
    for (i = 0; i < 100000; i++) {
        
        pbig[i] = (big_t *)mem_alloc(sizeof(big_t));
        printf("i = %d\n", i);
        pbig[i]->arr[1024] = 'c';
        pbig[i]->num = i;
    }
    printf("memory already allocated\n");
    for (i = 0; i < 100000; i++) {
        printf("pbig[%d] %c %d\n", i, pbig[i]->arr[1024], pbig[i]->num);
    }
    for (i = 0; i < 100000; i++) {
        mem_free(pbig[i], sizeof(big_t));
    }
    printf("end memory allocate\n");
}



// #endif