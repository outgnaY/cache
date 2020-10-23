/* memory allocate */
#include "cache.h"
typedef struct {
    unsigned int size;      /* size of items */
    unsigned int perslab;   /* how many items per slab */
    void *slots;            /* list of free items */
    unsigned int sl_curr;   /* total free items in list */
    unsigned int slabs;     /* how many slabs were allocated for this class */
    void **slab_list;       /* array of slab pointers */
    unsigned int list_size; /* size of slab list */
} slabclass_t;

static slabclass_t slabclass[MAX_NUMBER_OF_SLAB_CLASSES];

static int power_largest;

/* 
 * find which slab class is required to store an item of a given size
 * 0 means error: can't store such a large object
 */
unsigned int slabs_clsid(const int size) {
    int res = POWER_SMALLEST;
    if (size == 0 || size > settings.item_size_max) {
        return 0;
    }
    while(size > slabclass[res].size) {
        if (res++ == power_largest) {
            return power_largest;
        }
    }
    return res;
}

unsigned int slabs_size(const int clsid) {
    return slabclass[clsid].size;
}

static void split_slab_page_into_freelist(char *ptr, const unsigned int id) {
    slabclass_t *p = &slabclass[id];
    int x;
    for (x = 0; x < p->perslab; x++) {
        do_slabs_free(ptr, 0, id);
        ptr += p->size;
    }
}

static void do_slabs_free(void *ptr, const int size, unsigned int id) {
    slabclass_t *p;
    
}