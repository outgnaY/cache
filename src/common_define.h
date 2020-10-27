/* common defines */
#define STRERROR(no) (strerror(no) != NULL ? strerror(no) : "unknown error")
#define VERSION 1.0
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
#define ROUND8(x)   (((x) + 7) & ~7)