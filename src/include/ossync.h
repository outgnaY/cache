#include "common.h"

typedef pthread_mutex_t os_fast_mutex_t;

void os_fast_mutex_init(os_fast_mutex_t *mutex);

void os_fast_mutex_lock(os_fast_mutex_t *mutex);

void os_fast_mutex_unlock(os_fast_mutex_t *mutex);

void os_fast_mutex_free(os_fast_mutex_t *mutex);