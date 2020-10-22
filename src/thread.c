/* thread management */
#include <pthread.h>
#include "cache.h"

#define ITEMS_PER_ALLOC 64
extern struct settings settings;
/* an item in the connection queue */
typedef struct conn_queue_item CQ_ITEM;
struct conn_queue_item {
    int sfd;                        /* socket fd */
    enum conn_states init_state;    /* init_state */
    int event_flags;                /* event_flags */
    CQ_ITEM *next;
};

/* a connection queue */
typedef struct conn_queue CQ;
struct conn_queue {
    CQ_ITEM *head;
    CQ_ITEM *tail;
    pthread_mutex_t lock;
};

/* free list of CQ_ITEM structs */
static CQ_ITEM *cq_item_freelist;
static pthread_mutex_t cq_item_freelist_lock;

/* libevent threads */
static LIBEVENT_THREAD *threads;
/*
 * init lock for worker threads
 */
static int init_count = 0;
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;

/* initializes a connection queue */
static void cq_init(CQ *cq) {
    pthread_mutex_init(&cq->lock, NULL);
    cq->head = NULL;
    cq->tail = NULL;
}

/* 
 * pop an item on a connection queue
 * returns the item, or NULL if no item is available
 */
static CQ_ITEM *cq_pop(CQ *cq) {
    CQ_ITEM *item;
    pthread_mutex_lock(&cq->lock);
    item = cq->head;
    if (item != NULL) {
        cq->head = item->next;
        if (NULL == cq->head) {
            cq->tail = NULL;
        }
    }
    pthread_mutex_unlock(&cq->lock);
    return item;
}

/* push an item to a connecion queue */
static void cq_push(CQ *cq, CQ_ITEM *item) {
    item->next = NULL;
    pthread_mutex_lock(&cq->lock);
    if (cq->tail == NULL) {
        cq->head = item;
    } else {
        cq->tail->next = item;
    }
    cq->tail = item;
    pthread_mutex_unlock(&cq->lock);
}

/* returns a new connection queue item */
static CQ_ITEM *cq_new_item(void) {
    CQ_ITEM *item = NULL;
    pthread_mutex_lock(&cq_item_freelist_lock);
    if (cq_item_freelist) {
        item = cq_item_freelist;
        cq_item_freelist = item->next;
    }
    pthread_mutex_unlock(&cq_item_freelist_lock);
    if (item == NULL) {
        int i;
        /* allocate a batch of items */
        item = malloc(sizeof(CQ_ITEM) * ITEMS_PER_ALLOC);
        if (item == NULL) {
            // TODO statistic
            return NULL;
        }
        /* link all the new allocated items except the first one */
        for (i = 2; i < ITEMS_PER_ALLOC; i++) {
            item[i - 1].next = &item[i];
        }
        pthread_mutex_lock(&cq_item_freelist_lock);
        item[ITEMS_PER_ALLOC - 1].next = cq_item_freelist;
        cq_item_freelist = &item[1];
        pthread_mutex_unlock(&cq_item_freelist_lock);
    }
    return item;
}

/* frees a connection queue item (adds it to the freelist) */
static void cq_free_item(CQ_ITEM *item) {
    pthread_mutex_lock(&cq_item_freelist_lock);
    item->next = cq_item_freelist;
    cq_item_freelist = item;
    pthread_mutex_unlock(&cq_item_freelist_lock);
}

/******************** libevent threads ********************/

static void wait_for_thread_registration(int nthreads) {
    while (init_count < nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
}

static void register_thread_initialized(void) {
    pthread_mutex_lock(&init_lock);
    init_count++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);
}

/* creates a worker thread */
static void create_worker(void *(*func)(void *), void *arg) {
    pthread_attr_t attr;
    int ret;
    pthread_attr_init(&attr);
    if ((ret = pthread_create(&((LIBEVENT_THREAD*)arg)->thread_id, &attr, func, arg)) != 0) {
        fprintf(stderr, "can't create thread: %s\n", strerror(ret));
        exit(1);
    }
}

/*
 * process an incoming item.
 * called when input arrives on the libevent wakeup pipe
 */
static void thread_libevent_process(evutil_socket_t fd, short which, void *arg) {
    LIBEVENT_THREAD *this = arg;
    CQ_ITEM *item;
    char buf[1];

    if (read(fd, buf, 1) != 1) {
        fprintf(stderr, "can't read from libevent pipe\n");
        return;
    }
    switch (buf[0]) {
    /* a new conenction arrives */
    case 'c':
        /* 
         * allocate the item
         * assume that we can fetch an item by cq_pop method, cause we allocate it in dispatch_conn_new method
         */
        item = cq_pop(this->new_conn_queue);
        if (item == NULL) {
            break;
        }
        /* setup a new connection */
        
        /* free the item */
        cq_free_item(item);
        break;
    default:
        break;
    }
}

/* which thread we assigned a connection to most recently */
static int last_thread = -1;

/*
 * dispatches a new connection to a worker thread
 * only called from the main thread
 */
void dispatch_conn_new(int sfd, enum conn_states init_state, int event_flags) {
    /* allocate new item */
    CQ_ITEM *item = cq_new_item();
    char buf[1];
    if (item == NULL) {
        close(sfd);
        fprintf(stderr, "failed to allocate memory for connection object\n");
        return;
    }
    /* use round robin algorithm to select a thread */
    int tid = (last_thread + 1) % settings.num_threads;
    LIBEVENT_THREAD *thread = threads + tid;
    last_thread = tid;
    /* init item */
    item->sfd = sfd;
    item->init_state = init_state;
    item->event_flags = event_flags;

    cq_push(thread->new_conn_queue, item);
    buf[0] = 'c';
    /* notify worker thread */
    if (write(thread->notify_send_fd, buf, 1) != 1) {
        perror("writing to thread notify pipe");
    }
}

/* setup a thread's information */
static void setup_thread(LIBEVENT_THREAD *this) {
    // determine the init api according to libevent library version
#if defined(LIBEVENT_VERSION_NUMBER) && LIBEVENT_VERSION_NUMBER >= 0x2000101
    struct event_config *ev_config;
    ev_config = event_config_new();
    event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);
    this->base = event_base_new_with_config(ev_config);
    event_config_free(ev_config);    
#else
    this->base = event_init();
#endif
    if (!this->base) {
        fprintf(stderr, "can't allocate event base\n");
        exit(1);
    }
    /* listen for notifications from other threads */
    event_set(&this->notify_event, this->notify_receive_fd, EV_READ | EV_PERSIST, thread_libevent_process, this);
    event_base_set(this->base, &this->notify_event);

    if (event_add(&this->notify_event, 0) == -1) {
        fprintf(stderr, "can't monitor libevent notify pipe\n");
        exit(1);
    }

    this->new_conn_queue = malloc(sizeof(struct conn_queue));
    if (this->new_conn_queue == NULL) {
        perror("failed to allocate memory for conenction queue");
        exit(EXIT_FAILURE);
    }
    cq_init(this->new_conn_queue);

}

/* worker thread mainloop */
static void *worker_mainloop(void *arg) {
    LIBEVENT_THREAD *this = arg;
    // TODO logger
    register_thread_initialized();
    event_base_loop(this->base, 0);
    // watch for all threads exiting
    register_thread_initialized();
    event_base_free(this->base);
    return NULL;
}

/* initializes the threads */
void cache_thread_init(int nthreads, void *arg) {
    int i;
    /* init lock for worker threads */
    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);
    /* connection queue freelist lock */
    pthread_mutex_init(&cq_item_freelist_lock, NULL);
    cq_item_freelist = NULL;
    /* setup worker threads */
    threads = calloc(nthreads, sizeof(LIBEVENT_THREAD));
    if (!threads) {
        perror("Can't allocate thread descriptors");
        exit(1);
    }
    for (i = 0; i < nthreads; i++) {
        int fds[2];
        if (pipe(fds)) {
            perror("can't create notify pipe");
            exit(1);
        }
        threads[i].notify_receive_fd = fds[0];
        threads[i].notify_send_fd = fds[1];
        setup_thread(&threads[i]);
    }
    /* create worker threads */
    for (i = 0; i < nthreads; i++) {
        create_worker(worker_mainloop, &threads[i]);
    }
    /* wait for all the threads to set themselves up before returning */
    pthread_mutex_lock(&init_lock);
    wait_for_thread_registration(nthreads);
    pthread_mutex_unlock(&init_lock);
}