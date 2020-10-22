
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#include <event.h>

#include "connection.h"
/* slab sizing definitions */
#define POWER_SMALLEST 1

/* max number of slab classes */
#define MAX_NUMBER_OF_SLAB_CLASSES (63 + 1)

/* global settings */
struct settings {
    int item_size_max;      /* maximum item size */
    int num_threads;        /* number of worker threads */
};

/* stored item structure */
typedef struct item {
    struct item *next;
    struct item *prev;
    uint8_t slabs_clsid;

} item;

/* forward declaration */
typedef struct conn_queue CQ;

typedef struct libevent_thread LIBEVENT_THREAD;
struct libevent_thread {
    pthread_t thread_id;                /* unique ID of this thread */
    struct event_base *base;            /* libevent handle this thread uses */
    struct event notify_event;          /* listen event for notify pipe */
    int notify_receive_fd;              /* receiving end of notify pipe */
    int notify_send_fd;                 /* sending end of notify pipe */
    CQ *new_conn_queue;  /* queue of new connections */
};


/* 
 * functions related to connection
 */


/*
 * functions that are libevent-related
 */
void cache_thread_init(int nthreads, void *arg);
void dispatch_conn_new(int sfd, enum conn_states init_state, int event_flags);

/*
 * result codes
 */
#define CACHE_OK 0                          /* successful result */

/*
 * round up a number to the next larger multiple of 8.
 * used to force 8-byte alignment on 64-bit architectures.
 */
#define ROUND8(X)   (((x) + 7) & ~7)

/*
 * functions
 */
extern int daemonize();



/*
 * memory allocation methods
 */
typedef struct cache_mem_methods cache_mem_methods;
struct cache_mem_methods {
    void *(*mem_malloc)(int);               /* memory allocation function */
    void *(*mem_free)(void *);              /* memory free function */
    void *(*mem_realloc)(void *, int);      /* memory resize function */
    int (*mem_size)(void *);                /* return the size of an allocation */
    int (*mem_roundup)(int);                /* round up the request size to allocation size */
    int (*mem_init)(void *);                /* initialize the memory allocator */
    int (*mem_shutdown)(void *);            /* finalize the memory allocator */
};