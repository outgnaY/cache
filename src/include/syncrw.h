#ifndef SYNCRW_H
#define SYNCRW_H

#include "common.h"
#include "list.h"
#include "osthread.h"

#define RW_LOCK_NOT_LOCKED 350
#define RW_LOCK_EX 351
#define RW_LOCK_SHARED 352
#define RW_LOCK_WAIT_EX 353
#define SYNC_MUTEX 354

struct rw_lock_struct {
    ulint reader_count;             /* number of readers */
    ulint writer;                   /*  */
    os_thread_id_t writer_thread;   /* writer thread id */
    ulint writer_count;             /* number of time recursive locked */
    mutex_t mutex;                  /* mutex protecing rw lock */
    ulint waiters;                  /* this ulint is set to 1 if there are waiters (readers or writers) waiting for this rw_lock */
    bool writer_is_wait_ex;
    LIST_NODE_T(rw_lock_t) list;    /* all rw locks list */
    ulint level;                    /* latching order */
};  

typedef struct rw_lock_struct rw_lock_t;
typedef LIST_BASE_NODE_T(rw_lock_t) rw_lock_list_t;

extern rw_lock_list_t rw_lock_list;
extern mutex_t rw_lock_list_mutex;

/* statistics */
extern ulint rw_s_system_call_count;

void rw_lock_create_func(rw_lock_t *lock);

void rw_lock_free(rw_lock_t *lock);

bool rw_lock_validate(rw_lock_t *lock);

void rw_lock_x_lock_func(rw_lock_t *lock, ulint pass);

void rw_lock_x_unlock_func(rw_lock_t *lock);


#endif  /* SYNCRW_H */