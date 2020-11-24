#ifndef SYNCARR_H
#define SYNCARR_H
#include "common.h"
#include "ossync.h"

typedef struct sync_cell_struct sync_cell_t;
typedef struct sync_array_struct sync_array_t;

#define SYNC_ARRAY_OS_MUTEX 1
#define SYNC_ARRAY_MUTEX 2

sync_array_t *sync_array_create(ulint n_cells, ulint protection);

void sync_array_free(sync_array_t *arr);

void sync_array_reserve_cell(sync_array_t *arr, void *object, ulint type, ulint *index);

void sync_array_wait_event(sync_array_t *arr, ulint index);

void sync_array_free_cell(sync_array_t *arr, ulint index);

void sync_array_signal_object(sync_array_t *arr, void *object);

void sync_arr_wake_threads_if_sema_free();

struct sync_cell_struct {
    void *wait_object;
    /*mutex_t *old_wait_mutex;
    rw_lock_t *old_Wait_rw_lock;
    ulint request_type;*/
    os_thread_id_t thread;
    bool waiting;
    bool event_set;
    os_event_t event;
    time_t reservation_time;
};

struct sync_array_struct {
    ulint n_reserved;
    ulint n_cells;
    sync_cell_t *array;
    ulint protection;           /* which mutex protects the data */
    mutex_t mutex;
    os_mutex_t os_mutex;
    ulint sg_count;
    ulint res_count;

};

#endif  /* SYNCARR_H */