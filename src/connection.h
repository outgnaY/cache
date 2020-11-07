#ifndef CONNECTION_H
#define CONNECTION_H

// forward declarations 
typedef struct libevent_thread LIBEVENT_THREAD;
typedef unsigned int rel_time_t;
struct event;
// states of a connection 
typedef enum {
    conn_listening,
    conn_new_cmd,
    conn_read,
    conn_closing,
    conn_closed,
    conn_max_state,     // used to assert 
} conn_states;
// data structure representing a connection into cache 
typedef struct conn conn;
// export globals
extern conn **conns;                // connection array 
extern pthread_mutex_t conn_lock;   // connection lock
extern conn *listen_conn;


struct conn {
    int sfd;                            // socket fd of the connection 
    conn_states state;                  // state of the connection 
    struct event event;                 // libevent event 
    short event_flags;                  // event flags 
    LIBEVENT_THREAD *thread;            // pointer to the thread object serving this connection 
    rel_time_t last_cmd_time;           // time for the last command to this connection 
    short which;                        // which events were just triggered
    conn *next;                         // used for generating a list of conn structures
};

// init conns
void conn_init(void);

// create a new connection 
conn *conn_new(const int sfd, conn_states init_state, const short event_flags, struct event_base *base);

void accept_new_conns(const bool do_accept);

// close a connetion
void conn_close(conn *c);

// close idle connection 
void conn_close_idle(conn *c);

// close all connections
void conn_close_all(void);

// start thread for timeout checking
int start_conn_timeout_thread();

// stop thread for timeout checking 
int stop_conn_timeout_thread(void);



#endif