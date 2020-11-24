#include "ossync.h"
#include "utilmem.h"

/* create an event */
os_event_t os_event_create() {
    os_event_t event;
    event = ut_malloc(sizeof(struct os_event_struct));
    os_fast_mutex_init(&(event->os_mutex));
    pthread_cond_init(&(event->cond_var), NULL);
    event->is_set = FALSE;
    return event;
}

/* sets an event to the signaled state */
void os_event_set(os_event_t event) {
    assert(event != NULL);
    os_fast_mutex_lock(&(event->os_mutex));
    if (event->is_set) {
        /* do nothing */
    } else {
        event->is_set = TRUE;
        pthread_cond_broadcast(&(event->cond_var));
    }
    os_fast_mutex_unlock(&(event->os_mutex));
}

/* reset an event to the unsignaled state.waiting threads will stop to wait for the event */
void os_event_reset(os_event_t event) {
    assert(event != NULL);
    os_fast_mutex_lock(&(event->os_mutex));
    if (!event->is_set) {
        /* do nothing */
    } else {
        event->is_set = FALSE;
    }
    os_fast_mutex_unlock(&(event->os_mutex));
}

/* free an event */
void os_event_free(os_event_t event) {
    assert(event != NULL);
    os_fast_mutex_free(&(event->os_mutex));
    pthread_cond_destroy(&(event->cond_var));
    ut_free(event);
}

void os_event_wait(os_event_t event) {
    os_fast_mutex_lock(&(event->os_mutex));
    /* spin */
loop:
    if (event->is_set == TRUE) {
        os_fast_mutex_unlock(&(event->os_mutex));
        return;
    }
    pthread_cond_wait(&(event->cond_var), &(event->os_mutex));
    goto loop;
}

/* wait at most time microseconds */
ulint os_event_wait_time(os_event_t event, ulint time) {
    os_event_wait(event);
    return 0;
}

/* wait for any event in an event array. return if any one is signaled */
ulint os_event_wait_multiple(ulint n, os_event_t *event_array) {
    assert(n == 0);
    os_event_wait(*event_array);
    return 0;
}

os_mutex_t os_mutex_create() {
    os_fast_mutex_t *os_mutex;
    os_mutex_t mutex;
    os_mutex = ut_malloc(sizeof(os_fast_mutex_t));
    os_fast_mutex_init(os_mutex);
    mutex = ut_malloc(sizeof(os_mutex_struct_t));
    mutex->handle = os_mutex;
    mutex->count = 0;
    return mutex;
}

void os_mutex_enter(os_mutex_t mutex) {
    os_fast_mutex_lock(mutex->handle);
    (mutex->count)++;
}

void os_mutex_exit(os_mutex_t mutex) {
    (mutex->count)--;
    os_fast_mutex_unlock(mutex->handle);
}

void os_mutex_free(os_mutex_t mutex) {
    os_fast_mutex_free(mutex->handle);
    ut_free(mutex->handle);
    ut_free(mutex);
}


void os_fast_mutex_init(os_fast_mutex_t *mutex) {
    pthread_mutex_init(mutex, NULL);
}

ulint os_fast_mutex_trylock(os_fast_mutex_t *mutex) {
    return (ulint) pthread_mutex_trylock(mutex);
}

void os_fast_mutex_lock(os_fast_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
}

void os_fast_mutex_unlock(os_fast_mutex_t *mutex) {
    pthread_mutex_unlock(mutex);
}

void os_fast_mutex_free(os_fast_mutex_t *mutex) {
    
}