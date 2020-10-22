#include "cache.h"


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

/* close all connections */
void conn_close_all(void) {
    int i;
    /* 
    for (i = 0; i < ; i++) {
        
    }
    */
}

/* frees a connection */
void conn_free(conn *c) {

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

