#ifndef UTIL_H
#define UTIL_H

#include "common.h"

/**
 * @param n: number to be rounded
 * @param align_no: aligned by this number
 * @return rounded value
 */ 
ulint calc_align(ulint n, ulint align_no);

extern ulint ut_total_allocated_memory;

/* allocate memory */
void *ut_malloc_low(ulint n, bool set_to_zero);

/* allocate memory */
void *ut_malloc(ulint n);

/* free memory allocated with ut_malloc */
void ut_free(void *p);

/* free all allocated memory */
void ut_free_all_mem(void);

#endif  /* UTIL_H */