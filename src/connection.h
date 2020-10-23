#ifndef CONNECTION_H
#define CONNECTION_H

/* forward declarations */
typedef struct libevent_thread LIBEVENT_THREAD;
struct event;
/* states of a connection */
enum conn_states {
    conn_listening,
    conn_max_state,     /* used to assert */
};
/* data structure representing a connection into cache */
typedef struct conn conn;


struct conn {
    int sfd;                            /* socket fd of the connection */
    enum conn_states state;             /* state of the connection */
    struct event event;                 /* libevent event */
    short event_flags;                  /* event flags */
    LIBEVENT_THREAD *thread;            /* pointer to the thread object serving this connection */
    rel_time_t last_cmd_time;           /* time for the last command to this connection */
};

/* create a new connection */
conn *conn_new(const int sfd, const enum conn_states init_state, const short event_flags, struct event_base *base);

/* close idle connection */
void conn_close_idle(conn *c);

/* close all connections */
void conn_close_all(void);

/* stop connection timeout thread */
int stop_conn_timeout_thread(void);



#endif