#include "osthread.h"


os_thread_t os_thread_create(os_posix_f_t start_f, void *arg, os_thread_id_t *thread_id) {
    int ret;
    os_thread_t pthread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    ret = pthread_create(&pthread, &attr, start_f, arg);
    pthread_attr_destroy(&attr);
    return pthread;
}

os_thread_id_t os_thread_get_curr_id(void) {
    pthread_t pthr;
    pthr = pthread_self();
    return (*(os_thread_id_t *)((void *)(&pthr)));
}

os_thread_t os_thread_get_curr(void) {
    return pthread_self();
}

ulint os_thread_conv_id_to_ulint(os_thread_id_t id) {
    return (ulint)id;
}

void os_thread_yield(void) {
    sched_yield();
}

void os_thread_sleep(ulint tm) {
    struct timeval t;
    t.tv_sec = tm / 1000000;
    t.tv_usec = tm % 1000000;
    select(0, NULL, NULL, NULL, &t);
}