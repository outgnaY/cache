#ifndef OS_SYNC_H
#define OS_SYNC_H

#include "common.h"

typedef pthread_mutex_t os_fast_mutex_t;
struct os_event_struct {
    os_fast_mutex_t os_mutex;       /* mutex protects the struct */
    bool is_set;                    /* TRUE if the next mutex is not reserved */
    pthread_cond_t cond_var;        /* condition variable used in waiting for the event */
};

typedef struct os_event_struct os_event_struct_t;
typedef os_event_struct_t* os_event_t;

struct os_mutex_struct {
    void *handle;                   /* os handle to mutex */
    ulint count;                    /* check recursive lock */
};

typedef struct os_mutex_struct os_mutex_struct_t;
typedef os_mutex_struct_t* os_mutex_t;

os_event_t os_event_create();

void os_event_set(os_event_t event);

void os_event_reset(os_event_t event);

void os_event_free(os_event_t event);

void os_event_wait(os_event_t event);

ulint os_event_wait_time(os_event_t event, ulint time);

ulint os_event_wait_multiple(ulint n, os_event_t *event_array);

os_mutex_t os_mutex_create();

void os_mutex_enter(os_mutex_t mutex);

void os_mutex_exit(os_mutex_t mutex);

void os_mutex_free(os_mutex_t mutex);

void os_fast_mutex_init(os_fast_mutex_t *mutex);

ulint os_fast_mutex_trylock(os_fast_mutex_t *mutex);

void os_fast_mutex_lock(os_fast_mutex_t *mutex);

ulint os_fast_mutex_trylock(os_fast_mutex_t *mutex);

void os_fast_mutex_unlock(os_fast_mutex_t *mutex);

void os_fast_mutex_free(os_fast_mutex_t *mutex);


#endif  /* OSSYNC_H */