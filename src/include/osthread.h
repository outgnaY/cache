#ifndef OSTHREAD_H
#define OSTHREAD_H

#include "common.h"

#define OS_THREAD_MAX_N 1000

typedef pthread_t os_thread_t;
typedef unsigned long int os_thread_id_t;

typedef void* (*os_posix_f_t) (void *);

os_thread_t os_thread_create(os_posix_f_t start_f, void *arg, os_thread_id_t *thread_id);


os_thread_id_t os_thread_get_curr_id(void);

os_thread_t os_thread_get_curr(void);

ulint os_thread_conv_id_to_ulint(os_thread_id_t id);

void os_thread_yield(void);

void os_thread_sleep(ulint tm);




#endif  /* OSTHREAD_H */