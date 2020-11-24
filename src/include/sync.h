#ifndef SYNC_H
#define SYNC_H

#include "common.h"
#include "list.h"
#include "ossync.h"
#include "osthread.h"

/* number of slots reserved for each OS thread in the sync level array */
#define SYNC_THREAD_N_LEVELS 10000

#define SYNC_NO_ORDER_CHECK 3000
#define SYNC_LEVEL_NONE 2000

typedef struct mutex_struct mutex_t;

typedef struct sync_level_struct sync_level_t;
typedef struct sync_thread_struct sync_thread_t;

struct mutex_struct {
    ulint lock_word;
    os_fast_mutex_t os_fast_mutex;
    ulint waiters;
    LIST_NODE_T(mutex_t) list;
    os_thread_id_t thread_id;
    ulint level;
};

struct sync_thread_struct {
    os_thread_id_t id;
    sync_level_t *levels;
};

struct sync_level_struct {
    void *latch;    /* pointer to a mutex or an rw-lock; NULL means the slot is empty */
    ulint level;
};


void mutex_create_func(mutex_t *mutex);

void mutex_free(mutex_t *mutex);



#endif  /* SYNC_H */