#include "sync.h"
#include "syncarr.h"

ulint mutex_spin_round_count = 0;
ulint mutex_spin_wait_count = 0;
ulint mutex_os_wait_count = 0;
ulint mutex_exit_count = 0;

/* global array of wait cells */
sync_array_t *sync_primary_wait_array;

bool sync_initialized = FALSE;

/* list of mutexes */
LIST_BASE_NODE_T(mutex_t) mutex_list;

/* mutex protecting the mutex_list */
mutex_t mutex_list_mutex;

sync_thread_t *sync_thread_level_arrays;

mutex_t sync_thread_mutex;

static inline void mutex_set_waiters(mutex_t *mutex, ulint n) {
    volatile ulint *p;
    p = &(mutex->waiters);
    *p = n;
}

static inline ulint mutex_test_and_set(mutex_t *mutex) {
    bool ret;
    ret = os_fast_mutex_trylock(&(mutex->os_fast_mutex));
    if (ret == 0) {
        mutex->lock_word = 1;
    }
    return ret;
}

static inline void mutex_reset_lock_word(mutex_t *mutex) {
    mutex->lock_word = 0;
    os_fast_mutex_unlock(&(mutex->os_fast_mutex));
}

static inline ulint mutex_get_lock_word(mutex_t *mutex) {
    /* ensure lock_word is loaded from memory */
    volatile ulint *p;
    p = &(mutex->lock_word);
    return *p;
}

static inline ulint mutex_get_waiters(mutex_t *mutex) {
    volatile ulint *p;
    p = &(mutex->waiters);
    return *p;
}

static inline void mutex_exit(mutex_t *mutex) {
    mutex_reset_lock_word(mutex);
    if (mutex_get_waiters(mutex) != 0) {
        mutex_signal_object(mutex);
    }
}

static inline void mutex_enter_func(mutex_t *mutex) {
    if (!mutex_test_and_set(mutex)) {
        return;
    }
    mutex_spin_wait(mutex);
}

void mutex_create_func(mutex_t *mutex) {
    os_fast_mutex_init(&(mutex->os_fast_mutex));
    mutex->lock_word = 0;
    mutex_set_waiters(mutex, 0);
    mutex->thread_id = ULINT_UNDEFINED;
    mutex->level = SYNC_LEVEL_NONE;
    assert(((ulint)(&(mutex->lock_word))) % 4 == 0);
    mutex_enter(&mutex_lis)
}


void mutex_free(mutex_t *mutex) {
    mutex_enter(&mutex_list_mutex);
    LIST_REMOVE(list, mutex_list, mutex);
    mutex_exit(&mutex_list_mutex);
    os_fast_mutex_free(&(mutex->os_fast_mutex));
}

ulint mutex_enter_nowait(mutex_t *mutex) {
    if (!mutex_test_and_set(mutex)) {
        /* success */
        return 0;
    }
    return 1;
}

void mutex_set_waiters(mutex_t *mutex, ulint n) {
    volatile ulint *p;
    p = &(mutex->waiters);
    *p = n;
}

void mutex_spin_wait(mutex_t *mutex) {
    ulint index;
    ulint i;
mutex_loop:
    i = 0;
spin_loop:

}

void mutex_signal_object(mutex_t *mutex) {
    mutex_set_waiters(mutex, 0);
    sync_array_signal_object(sync_primary_wait_array, mutex);
}

void mutex_set_level(mutex_t *mutex, ulint level) {
    mutex->level = level;
}

bool mutex_own(mutex_t *mutex) {
    if (mutex_get_lock_word(mutex) != 1) {
        return FALSE;
    }
    if (mutex->thread_id != os_thread_get_curr_id()) {
        return FALSE;
    }
    return TRUE;
}

static sync_thread_t *sync_thread_level_arrays_get_nth(ulint n) {
    return sync_thread_level_arrays + n;
}

static sync_thread_t *sync_thread_level_arrays_find_slot() {
    sync_thread_t *slot;
    os_thread_id_t id;
    ulint i;
    id = os_thread_get_curr_id();
    for (i = 0; i < OS_THREAD_MAX_N; i++) {
        slot = sync_thread_level_arrays_get_nth(i);
        if (slot->levels && (slot->id == id)) {
            return slot;
        }
    }
    return NULL;
}

static sync_thread_t *sync_thread_level_arrays_find_free() {
    sync_thread_t *slot;
    ulint i;
    for (i = 0; i < OS_THREAD_MAX_N; i++) {
        slot = sync_thread_level_arrays_get_nth(i);
        if (slot->levels == NULL) {
            return slot;
        }
    }
    return NULL;
}

static sync_level_t *sync_thread_levels_get_nth(sync_level_t *arr, ulint n) {
    return arr + n;
}

/* checks if all the level values stored in the level array are greater than the given limit */
static bool sync_thread_levels_g(sync_level_t *arr, ulint limit) {

}

static bool sync_thread_levels_contain(sync_level_t *arr, ulint level) {
    sync_level_t *slot;
    ulint i;
    for (i = 0; i < SYNC_THREAD_N_LEVELS; i++) {
        slot = sync_thread_levels_get_nth(arr, i);
        if (slot->latch != NULL) {
            if (slot->level == level) {
                return TRUE;
            }
        }
    }
    return FALSE;
}


void sync_init() {
    sync_thread_t *thread_slot;
    ulint i;
    sync_initialized = TRUE;
    sync_primary_wait_array = sync_array_create(OS_THREAD_MAX_N, SYNC_ARRAY_OS_MUTEX);

}

void sync_close() {
    sync_array_free(sync_primary_wait_array);
}




