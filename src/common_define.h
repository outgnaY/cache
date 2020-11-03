/* common defines */
#define VERSION 1.0

typedef enum {
    false = 0,
    true = 1
} bool;
#define STRERROR(no) (strerror(no) != NULL ? strerror(no) : "unknown error")
/* slab sizing definitions */
#define POWER_SMALLEST 1

/* max number of slab classes */
#define MAX_NUMBER_OF_SLAB_CLASSES (63 + 1)

/*
 * result codes
 */
#define CACHE_OK 0                          /* successful result */

/*
 * round up a number to the next larger multiple of 8.
 * used to force 8-byte alignment on 64-bit architectures.
 */
#define ROUND8(n)   (((n) + 7) & ~7)
#define ROUNDUP(n, align) ((((n) + (align) - 1) & ~((align) - 1)))