#include "cache.h"

// forward declarations 
static void conn_cleanup(conn *c);
static void conn_free(conn *c);
static void *conn_timeout_thread(void *arg);

// static variables 
static int max_fds;                                 // maximum fds 
static volatile bool do_run_conn_timeout_thread;     // timeout thread run flag 
static pthread_t conn_timeout_tid;                  // thread id of timeout thread 
static volatile bool allow_new_conns = true;        // controls if we allow new connections
static struct event maxconnsevent;

// exported globals 
conn **conns;                                           // connection array
pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;   
conn *listen_conn = NULL;

static void maxconns_handler(const evutil_socket_t fd, const short which, void *arg) {
    struct timeval t = {.tv_sec = 0, .tv_usec = 10000};
    if (fd == -42 || allow_new_conns == false) {
        // reschedule in 10ms if we need to keep polling
        evtimer_set(&maxconnsevent, maxconns_handler, 0);
        event_base_set(main_base, &maxconnsevent);
        evtimer_add(&maxconnsevent, &t);
    } else {
        evtimer_del(&maxconnsevent);
        accept_new_conns(true);
    }
}

void accept_new_conns(const bool do_accept) {
    printf("accept new conns\n");
    pthread_mutex_lock(&conn_lock);
    do_accept_new_conns(do_accept);
    pthread_mutex_unlock(&conn_lock);
}

// accept new connections
void do_accept_new_conns(const bool do_accept) {
    conn *next;
    for (next = listen_conn; next; next = next->next) {
        if (do_accept) {
            update_event(next, EV_READ | EV_PERSIST);
            if (listen(next->sfd, settings.backlog) != 0) {
                perror("listen");
            }
        } else {
            update_event(next, 0);
            if (listen(next->sfd, 0) != 0) {
                perror("listen");
            }
        }
    }
    if (do_accept) {

    } else {
        allow_new_conns = false;
        maxconns_handler(-42, 0, 0);
    }
}
  

// create a new connection
conn *conn_new(const int sfd, conn_states init_state, const short event_flags, struct event_base *base) {
    conn *c;
    printf("conn_new: fd = %d\n", sfd);
    assert(sfd >= 0 && sfd < max_fds);
    c = conns[sfd];
    if (c == NULL) {
        if (!(c = (conn *)calloc(1, sizeof(conn)))) {
            fprintf(stderr, "failed to allocate connection object\n");
        }
        c->sfd = sfd;
        conns[sfd] = c;
    }
    /*
    if (init_state == conn_new_cmd) {

    }
    */
    c->state = init_state;
    // initialize for idle kicker
    c->last_cmd_time = g_rel_current_time;
    // setup event 
    event_set(&c->event, sfd, event_flags, event_handler, (void *)c);
    event_base_set(base, &c->event);
    c->event_flags = event_flags;
    if (event_add(&c->event, 0) == -1) {
        perror("event add");
        return NULL;
    }
    return c;
}

// clean up a connection 
static void conn_cleanup(conn *c) {
    assert(c != NULL);
}


// close a connection 
void conn_close(conn *c) {
    assert(c != NULL);
    // delete the event, the socket and the conn
    event_del(&c->event);
    if (settings.verbose > 1) {
        fprintf(stderr, "connection closed, fd = %d\n", c->sfd);
    }
    // do clean jobs
    conn_cleanup(c);
    conn_set_state(c, conn_closed);
    close(c->sfd);
    pthread_mutex_lock(&conn_lock);
    allow_new_conns = true;
    pthread_mutex_unlock(&conn_lock);
    
    return;
}

// close idle connection 
void conn_close_idle(conn *c) {
    if (settings.idle_timeout > 0 && (g_rel_current_time - c->last_cmd_time) > settings.idle_timeout) {
        // a connection timeout 
        if (c->state != conn_new_cmd && c->state != conn_read) {
            if (settings.verbose > 0) {
                fprintf(stderr, "fd %d wants to timeout, but isn't in read state\n", c->sfd);
            }
            return;
        }
        if (settings.verbose > 0) {
            fprintf(stderr, "closing idle fd %d\n", c->sfd);
        }
        conn_set_state(c, conn_closing);
        drive_machine(c);
    }
}

// close all connections 
void conn_close_all(void) {
    int i;
    for (i = 0; i < max_fds; i++) {
        if (conns[i] && conns[i]->state != conn_closed) {
            conn_close(conns[i]);
        }
    }
}

// initializes the connections array 
void conn_init(void) {
    int next_fd = dup(1);
    if (next_fd < 0) {
        perror("failed to duplicate file descriptor");
        exit(1);
    }
    int headroom = 10;          // extra room for unexpected open fds 
    struct rlimit rl;
    max_fds = settings.maxconns + headroom + next_fd;
    // get the actual highest fd if possible 
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

// frees a connection 
static void conn_free(conn *c) {
    if (c) {
        assert(c != NULL);
        assert(c->sfd >= 0 && c->sfd < max_fds);
        conns[c->sfd] = NULL;
        free(c);
    }
}

#define CONNS_PER_SLICE 100
#define TIMEOUT_MSG_SIZE (1 + sizeof(int))
// thread to kick out timeout threads 
static void *conn_timeout_thread(void *arg) {
    int i;
    conn *c;
    char buf[TIMEOUT_MSG_SIZE];
    rel_time_t oldest_last_cmd;
    int sleep_time;
    int sleep_slice = max_fds / CONNS_PER_SLICE;
    if (sleep_slice == 0) {
        sleep_slice = CONNS_PER_SLICE;
    }
    useconds_t timeslice = 1000000 / sleep_slice;
    while (do_run_conn_timeout_thread) {
        oldest_last_cmd = g_rel_current_time;
        for (i = 0; i < max_fds; i++) {
            // sleep for a while
            if ((i % CONNS_PER_SLICE) == 0) {
                usleep(timeslice);
            }
            if (!conns[i]) {
                continue;
            }
            c = conns[i];
            if (c->state != conn_new_cmd && c->state != conn_read) {
                continue;
            }
            // send connection timeout message
            if ((g_rel_current_time - c->last_cmd_time) > settings.idle_timeout) {
                buf[0] = 't';
                memcpy(&buf[1], &i, sizeof(int));
                if (write(c->thread->notify_send_fd, buf, TIMEOUT_MSG_SIZE) != TIMEOUT_MSG_SIZE) {
                    perror("failed to write timeout message to notify pipe");
                }
            } else {
                if (c->last_cmd_time < oldest_last_cmd) {
                    oldest_last_cmd = c->last_cmd_time;
                }
            }
        }
        // soonest we could have another connection timeout
        sleep_time = settings.idle_timeout - (g_rel_current_time - oldest_last_cmd) + 1;
        if (sleep_time <= 0) {
            sleep_time = 1;
        }
        usleep((useconds_t) sleep_time * 1000000);
    }
    return NULL;
}

int start_conn_timeout_thread() {
    int ret;
    if (settings.idle_timeout == 0) {
        return -1;
    }
    do_run_conn_timeout_thread = true;
    if ((ret = pthread_create(&conn_timeout_tid, NULL, conn_timeout_thread, NULL)) != 0) {
        fprintf(stderr, "can't create idle connection timeout thread: %s\n", STRERROR(ret));
        return -1;
    }
    return 0;
}

int stop_conn_timeout_thread(void) {
    if (!do_run_conn_timeout_thread) {
        return -1;
    }
    do_run_conn_timeout_thread = false;
    pthread_join(conn_timeout_tid, NULL);
    return 0;
}



// convert a state name to a human readable form 
/*
static const char *state_text(conn_states state) {
    const char* const statenames[] = {
        "conn_listening"
    };
    return statenames[state];
}
*/
// sets a conenction's current state in the state machine 
void conn_set_state(conn *c, conn_states state) {
    assert(c != NULL);
    assert(state >= conn_listening && state < conn_max_state);
    if (state != c->state) {
        // TODO log
        c->state = state;
    }
}

