
#include <stdint.h>

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