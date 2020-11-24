#include "syncrw.h"
#include "sync.h"

/* global list of rw locks */
rw_lock_list_t rw_lock_list;
mutex_t rw_lock_list_mutex;

/* getters and setters */
static inline ulint rw_lock_get_waiters(rw_lock_t *lock) {
    return lock->waiters;
}

static inline void rw_lock_set_waiters(rw_lock_t *lock, ulint flag) {
    lock->waiters = flag;
}

static inline ulint rw_lock_get_writer(rw_lock_t *lock) {
    return lock->writer;
}

static inline void rw_lock_set_writer(rw_lock_t *lock, ulint flag) {
    lock->writer = flag;
}

static inline ulint rw_lock_get_reader_count(rw_lock_t *lock) {
    return lock->reader_count;
}

static inline void rw_lock_set_reader_count(rw_lock_t *lock, ulint count) {
    lock->reader_count = count;
}

static inline ulint rw_lock_get_writer_count(rw_lock_t *lock) {
    return lock->writer_count;
}

static inline void rw_lock_set_writer_count(rw_lock_t *lock, ulint count) {
    lock->writer_count = count;
}

static inline mutex_t *rw_lock_get_mutex(rw_lock_t *lock) {
    return &(lock->mutex);
}

static inline ulint rw_lock_get_x_lock_count(rw_lock_t *lock) {
    return lock->writer_count;
}

static bool rw_lock_s_lock_low(rw_lock_t *lock, ulint pass) {
    /* TODO check if current thread owns this lock */
    if (lock->writer == RW_LOCK_NOT_LOCKED) {
        lock->reader_count++;
        return TRUE;
    }
    return FALSE;
}

static void rw_lock_s_lock_direct(rw_lock_t *lock) {
    lock->reader_count++;
}

static void rw_lock_x_lock_direct(rw_lock_t *lock) {
    rw_lock_set_writer(lock, RW_LOCK_EX);
    lock->writer_thread = ;
    lock->writer_count++;
    lock->pass = 0;
}

static void rw_lock_s_lock_func(rw_lock_t *lock, ulint pass) {
    mutex_enter(rw_lock_get_mutex(lock));
    if (rw_lock_s_lock_low(lock, pass)) {
        /* success */
        mutex_exit(rw_lock_get_mutex(lock));
        return;
    } else {
        /* not success, spin wait */
        mutex_exit(rw_lock_get_mutex(lock));
        rw_lock_s_lock_spin(lock, pass);
        return;
    }
}

static bool rw_lock_s_lock_func_nowait(rw_lock_t *lock) {
    bool success = FALSE;
    mutex_enter(rw_lock_get_mutex(lock));
    if (lock->writer == RW_LOCK_NOT_LOCKED) {
        lock->reader_count++;
        success = TRUE;
    }
    mutex_exit(rw_lock_get_mutex(lock));
    return success;
}

static bool rw_lock_x_lock_func_nowait(rw_lock_t *lock) {
    bool success = FALSE;
    mutex_enter(rw_lock_get_mutex(lock));
    /* no reader and no writer or locked by current thread */
    if ((rw_lock_get_reader_count(lock) == 0) && 
    ((rw_lock_get_writer(lock) == RW_LOCK_NOT_LOCKED) || ((rw_lock_get_writer(lock) == RW_LOCK_EX) && (lock->pass == 0) && (lock->writer_thread ==)))) {
        rw_lock_set_writer(lock, RW_LOCK_EX);
        lock->writer_thread = ;
        lock->writer_count++;
        lock->pass = 0;
        success = TRUE;
    }
    mutex_exit(rw_lock_get_mutex(lock));
    return success;
}

static void rw_lock_s_unlock_func(rw_lock_t *lock) {
    bool sg = FALSE;
    mutex_enter(rw_lock_get_mutex(lock));
    lock->reader_count--;
    /* there are still waiters wait for rw lock and no readers currently */
    if (lock->waiters && (lock->reader_count == 0)) {
        sg = TRUE;
        rw_lock_set_waiters(lock, 0);
    }
    mutex_exit(rw_lock_get_mutex(lock));
    if (sg == TRUE) {
        sync_array_signal_object(sync_primary_wait_array, lock);
    }
    mutex_exit(rw_lock_get_mutex(lock));
}


static void rw_lock_s_unlock_direct(rw_lock_t *lock) {
    lock->reader_count--;
}

static void rw_lock_x_unlock_func(rw_lock_t *lock) {
    bool sg = TRUE;
    mutex_enter(&(lock->mutex));
    lock->writer_count--;
    if (lock->writer_count == 0) {
        rw_lock_set_writer(lock, RW_LOCK_NOT_LOCKED);
    }
    /* there are still waiters wait for this rw lock and no writers currently */
    if (lock->waiters && (lock->writer_count == 0)) {
        sg = TRUE;
        rw_lock_set_waiters(lock, 0);
    }
    mutex_exit(&(lock->mutex));
    if (sg == TRUE) {
        sync_array_signal_object(sync_primary_wait_array, lock);
    }
}

static void rw_lock_x_unlock_direct(rw_lock_t *lock) {
    lock->writer_count--;
    if (lock->writer_count == 0) {
        rw_lock_set_writer(lock, RW_LOCK_NOT_LOCKED);
    }
}









void rw_lock_create_func(rw_lock_t *lock) {
    mutex_create(rw_lock_get_mutex(lock));
    mutex_set_level(rw_lock_get_mutex(lock), SYNC_NO_ORDER_CHECK);
    rw_lock_set_waiters(lock, 0);
    rw_lock_set_writer(lock, RW_LOCK_NOT_LOCKED);
    rw_lock_set_writer_count(lock, 0);
    rw_lock_set_reader_count(lock, 0);
    
}



void rw_lock_set_level(rw_lock_t *lock, ulint level) {
    lock->level = level;
}

bool rw_lock_is_locked(rw_lock_t *lock, ulint lock_type) {
    bool ret = FALSE;
    mutex_enter(&(lock->mutex));
    if (lock_type == RW_LOCK_SHARED) {
        if (lock->reader_count > 0) {
            ret = TRUE;
        }
    } else if (lock_type == RW_LOCK_EX) {
        if (lock->writer == RW_LOCK_EX) {
            ret = TRUE;
        }
    }
    mutex_exit(&(lock->mutex));
    return ret;
}

