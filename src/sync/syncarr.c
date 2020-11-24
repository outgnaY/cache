#include "syncarr.h"
#include "sync.h"
#include "ossync.h"
#include "syncrw.h"
#include "utilmem.h"


static sync_cell_t *sync_array_get_nth_cell(sync_array_t *arr, ulint n) {
    return arr->array + n;
}

static void sync_array_enter(sync_array_t *arr) {
    ulint protection;
    protection = arr->protection;
    if (protection == SYNC_ARRAY_OS_MUTEX) {
        os_mutex_enter(arr->os_mutex);
    } else if (protection == SYNC_ARRAY_MUTEX) {
        mutex_enter(&(arr->mutex));
    }
}

static void sync_array_exit(sync_array_t *arr) {
    ulint protection;
    protection = arr->protection;
    if (protection == SYNC_ARRAY_OS_MUTEX) {
        os_mutex_exit(arr->os_mutex);
    } else if (protection == SYNC_ARRAY_MUTEX) {
        mutex_exit(&(arr->mutex));
    }
}

sync_array_t *sync_array_create(ulint n_cells, ulint protection) {
    sync_array_t *arr;
    sync_cell_t *cell_array;
    sync_cell_t *cell;
    ulint i;
    arr = ut_malloc(sizeof(sync_array_t));
    cell_array = ut_malloc(sizeof(sync_cell_t) * n_cells);
    arr->n_cells = n_cells;
    arr->n_reserved = 0;
    arr->array = cell_array;
    arr->protection = protection;
    arr->sg_count = 0;
    arr->res_count = 0;
    if (protection == SYNC_ARRAY_OS_MUTEX) {
        arr->os_mutex = os_mutex_create();
    } else if (protection == SYNC_ARRAY_MUTEX) {
        mutex_create(&(arr->mutex));
        mutex_set_level(&(arr->mutex), SYNC_NO_ORDER_CHECK);
    }
    for (i = 0; i < n_cells; i++) {
        cell = sync_array_get_nth_cell(arr, i);
        cell->wait_object = NULL;
        cell->event = os_event_create();
        cell->event_set = FALSE;
    }
    return arr;
}

void sync_array_validate(sync_array_t *arr) {
    ulint i;
    sync_cell_t *cell;
    ulint count;
    count = 0;
    sync_array_enter(arr);
    for (i = 0; i < arr->n_cells; i++) {
        cell = sync_array_get_nth_cell(arr, i);
        if (cell->wait_object != NULL) {
            count++;
        }
    }
    assert(count == arr->n_reserved);
    sync_array_exit(arr);
}

static void sync_cell_event_set(sync_cell_t *cell) {
    os_event_set(cell->event);
    cell->event_set = TRUE;
}

static void sync_cell_event_reset(sync_cell_t *cell) {
    os_event_reset(cell->event);
    cell->event_set = FALSE;
}

void sync_array_reserve_cell(sync_array_t *arr, void *object, ulint type, ulint *index) {
    sync_cell_t *cell;
    ulint i;
    sync_array_enter(arr);
    arr->res_count++;
    /* reserve a new cell */
    for (i = 0; i < arr->n_cells; i++) {
        cell = sync_array_get_nth_cell(arr, i);
        if (cell->wait_object == NULL) {
            if (cell->event_set) {
                sync_cell_event_reset(cell);
            }
            cell->reservation_time = time(NULL);
            cell->thread = os_thread_get_curr_id();
            cell->wait_object = object;
            /*
            if (type == SYNC_MUTEX) {
                cell->old_wait_mutex = object;
            } else {
                cell->old_Wait_rw_lock = object;
            }
            cell->request_type = type;
            */
            cell->waiting = FALSE;
            arr->n_reserved++;
            *index = i;
            sync_array_exit(arr);
            return;
        }
        /* fail */
        assert(0);
    }
    return;
}

void sync_array_free(sync_array_t *arr) {
    ulint i;
    sync_cell_t *cell;
    ulint protection;
    sync_array_validate(arr);
    for (i = 0; i < arr->n_cells; i++) {
        cell = sync_array_get_nth_cell(arr, i);
        os_event_free(cell->event);
    }
    protection = arr->protection;
    if (protection == SYNC_ARRAY_OS_MUTEX) {
        os_mutex_free(arr->os_mutex);
    } else if (protection == SYNC_ARRAY_MUTEX) {
        mutex_free(&(arr->mutex));
    }
    ut_free(arr->array);
    ut_free(arr);
}

void sync_array_wait_event(sync_array_t *arr, ulint index) {
    sync_cell_t *cell;
    os_event_t event;
    sync_array_enter(arr);
    cell = sync_array_get_nth_cell(arr, index);
    event = cell->event;
    cell->waiting = TRUE;
    sync_array_exit(arr);
    os_event_wait(event);
    /* free the cell */
    sync_array_free_cell(arr, index);
}

static sync_cell_t *sync_array_find_thread(sync_array_t *arr, os_thread_id_t thread) {
    ulint i;
    sync_cell_t *cell;
    for (i = 0; i < arr->n_cells; i++) {
        cell = sync_array_get_nth_cell(arr, i);
        if ((cell->wait_object != NULL) && (cell->thread == thread)) {
            return cell;
        }
    }
    return NULL;
}

/*
static bool sync_arr_cell_can_wake_up(sync_cell_t *cell) {
    mutex_t *mutex;
    rw_lock_t *lock;
    if (cell->request_type == SYNC_MUTEX) {
        mutex = cell->wait_object;
        if (mutex_get_lock_word(mutex) == 0) {
            return TRUE;
        }
    } else if (cell->request_type == RW_LOCK_EX) {
        lock = cell->wait_object;
        if (rw_lock_get_reader_count(lock) == 0 && rw_lock_get_writer(lock) == RW_LOCK_NOT_LOCKED) {
            return TRUE;
        }
        if (rw_lock_get_reader_count(lock) == 0 && rw_lock_get_writer(lock) == RW_LOCK_WAIT_EX && lock->writer_thread == cell->thread) {
            return TRUE;
        }
    } else if (cell->request_type == RW_LOCK_SHARED) {
        lock = cell->wait_object;
        if (rw_lock_get_writer(lock) == RW_LOCK_NOT_LOCKED) {
            return TRUE;
        }
    }
    return FALSE;
}
*/

void sync_array_free_cell(sync_array_t *arr, ulint index) {
    sync_cell_t *cell;
    sync_array_enter(arr);
    cell = sync_array_get_nth_cell(arr, index);
    cell->wait_object = NULL;
    arr->n_reserved--;
    sync_array_exit(arr);
}

void sync_array_signal_object(sync_array_t *arr, void *object) {
    sync_cell_t *cell;
    ulint count;
    ulint i;
    sync_array_enter(arr);
    arr->sg_count++;
    i = 0;
    count = 0;
    while (count < arr->n_reserved) {
        cell = sync_array_get_nth_cell(arr, i);
        if (cell->wait_object != NULL) {
            count++;
            if (cell->wait_object == object) {
                sync_cell_event_set(cell);
            }
        }
        i++;
    }
    sync_array_exit(arr);
}