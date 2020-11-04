
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/resource.h>
#include <getopt.h>
#include <signal.h>
#include <sysexits.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <event.h>

#include "common_define.h"
#include "cache-config.h"
#include "connection.h"
#include "util.h"
#include "logger.h"
#include "list.h"


// relative time 
typedef unsigned int rel_time_t;

// enums 
typedef enum {
    NOT_STOP,
    // GRACE_STOP, 
    EXIT_NORMALLY
} stop_reasons;

typedef enum {
    CACHE_CONFIG_ALLOC
} cache_config_op;

// forward declarations 
typedef struct conn_queue CQ;
typedef struct cache_mem_methods cache_mem_methods;
typedef struct libevent_thread LIBEVENT_THREAD;

// global settings 
struct settings {
    int item_size_max;      // maximum item size 
    int num_threads;        // number of worker threads 
    int maxconns;           // max connections opened simultaneously 
    int idle_timeout;       // number of cseconds to let connections idle 
    cache_mem_methods *m;    // memory related methods
};
// exported globals 
extern struct settings settings;
extern volatile rel_time_t g_rel_current_time;        // how many seconds since process started 
extern volatile time_t g_current_time;                // global time stamp 

// stored item structure 
typedef struct item {
    struct item *next;
    struct item *prev;
    uint8_t slabs_clsid;

} item;


struct libevent_thread {
    pthread_t thread_id;                // unique ID of this thread 
    struct event_base *base;            // libevent handle this thread uses 
    struct event notify_event;          // listen event for notify pipe 
    int notify_receive_fd;              // receiving end of notify pipe 
    int notify_send_fd;                 // sending end of notify pipe 
    CQ *new_conn_queue;                 // queue of new connections 
};


/**
 * setup global configs
 */
int cache_config(cache_config_op op, ...);

void cache_mem_set_default(void);

/* 
 * functions related to connection
 */


/*
 * functions that are libevent-related
 */
void cache_thread_init(int nthreads, void *arg);
void dispatch_conn_new(int sfd, conn_states init_state, int event_flags);



/*
 * functions
 */
extern int daemonize();


/*
 * memory allocation methods
 */
struct cache_mem_methods {
    void *(*mem_malloc)(size_t);                        // memory allocation function 
    void *(*mem_free)(void *, size_t);                  // memory free function 
    void *(*mem_realloc)(void *, size_t, size_t);       // memory resize function 
    size_t (*mem_roundup)(size_t);                      // round up the request size to allocation size 
    int (*mem_init)(void *);                            // initialize the memory allocator 
    void (*mem_shutdown)(void *);                       // finalize the memory allocator 
};