#include "ossync.h"

void os_fast_mutex_init(os_fast_mutex_t *mutex) {
    pthread_mutex_init(mutex, NULL);
}

void os_fast_mutex_lock(os_fast_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
}

void os_fast_mutex_unlock(os_fast_mutex_t *mutex) {
    pthread_mutex_unlock(mutex);
}

void os_fast_mutex_free(os_fast_mutex_t *mutex) {
    
}