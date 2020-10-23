#include "cache.h"

/* forward declarations */
static void conn_cleanup(conn *c);
static void conn_close(conn *c);
static void conn_init(void);
static void conn_free(conn *c);
static void *conn_timeout_thread(void *arg);
static int start_conn_timeout_thread();

/* static variables */
static int max_fds;                                 /* maximum fds */
static volatile int do_run_conn_timeout_thread;     /* timeout thread run flag */
static pthread_t conn_timeout_tid;                  /* thread id of timeout thread */

/* exported globals */
conn **conns;

/* clean up a connection */
static void conn_cleanup(conn *c) {
    assert(c != NULL);
}


/* close a connection */
static void conn_close(conn *c) {
    assert(c != NULL);
    /* delete the event */
    event_del(&c->event);
    // TODO log

}

/* close idle connection */
void conn_close_idle(conn *c) {
    if (settings.idle_timeout > 0 && (current_time - c->last_cmd_time) > settings.idle_timeout) {
        /* a connection timeout */
        if (c->state)
    }
}

/* close all connections */
void conn_close_all(void) {
    int i;
    /* 
    for (i = 0; i < ; i++) {
        
    }
    */
}

/* initializes the connections array */
static void conn_init(void) {
    int next_fd = dup(1);
    if (next_fd < 0) {
        perror("failed to duplicate file descriptor");
        exit(1);
    }
    int headroom = 10;          /* extra room for unexpected open fds */
    struct rlimit rl;
    max_fds = settings.maxconns + headroom + next_fd;
    /* get the actual highest fd if possible */
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        max_fds = rl.rlim_max;
    } else {
        fprintf(stderr, "failed to query maximum file descriptor\n");
    }
    close(next_fd);
    if ((conns = calloc(max_fds, sizeof(conn *))) == NULL) {
        fprintf(stderr, "failed to allocate connection structures\n");
        exit(1);
    }
}

/* frees a connection */
static void conn_free(conn *c) {

}

/* thread to kick out timeout threads */
static void *conn_timeout_thread(void *arg) {

}

static int start_conn_timeout_thread() {
    int ret;
    if (settings.idle_timeout == 0) {
        return -1;
    }
    do_run_conn_timeout_thread = 1;
    if ((ret = pthread_create(&conn_timeout_tid, NULL, conn_timeout_thread, NULL)) != 0) {
        fprintf(stderr, "can't create idle connection timeout thread: %s\n", strerror(ret));
        return -1;
    }
    return 0;
}

int stop_conn_timeout_thread(void) {
    if (!do_run_conn_timeout_thread) {
        return -1;
    }
    do_run_conn_timeout_thread = 0;
    pthread_join(conn_timeout_tid, NULL);
    return 0;
}



/* convert a state name to a human readable form */
static const char *state_text(enum conn_states state) {
    const char* const statenames[] = {
        "conn_listening"
    };
    return statenames[state];
}

/* sets a conenction's current state in the state machine */
void conn_set_state(conn *c, enum conn_states state) {
    assert(c != NULL);
    assert(state >= conn_listening && state < conn_max_state);
    if (state != c->state) {
        // TODO log
        c->state = state;
    }
}

