#include "cache.h"

// exported globals 
struct settings settings;                       // settings 
volatile rel_time_t g_rel_current_time;         // current time, second
time_t process_started;                         // when the process was started
struct event_base *main_base;                   // main thread event base

// static variables 
static int stop_main_loop = NOT_STOP;
static volatile sig_atomic_t sig_hup = 0;            // a HUP signal received but not ye handled 
static struct event clockevent;

// init global settings
static void settings_init() {
    settings.verbose = 1;
    settings.num_threads = 4;
    settings.maxconns = 1024;
    settings.idle_timeout = 0;  // disabled
    settings.backlog = 1024;
    settings.maxconns_fast = true;
}

/**
 * setup global configs
 */
int cache_config(cache_config_op op, ...) {
    va_list ap;
    int rc = CACHE_OK;
    va_start(ap, op);
    switch (op) {
    case CACHE_CONFIG_ALLOC: {
        settings.m = va_arg(ap, cache_mem_methods *);
        break;
    }
    default: {
        break;
    }
    }
}


// remove pidfile 
static void remove_pidfile(const char *pid_file) {
    if (pid_file == NULL) {
        return;
    }
    if (unlink(pid_file) != 0) {
        fprintf(stderr, "could not remove the pid file %s\n", pid_file);
    }
}

static void sig_handler(const int sig) {
    stop_main_loop = EXIT_NORMALLY;
    printf("signal %s\n", strsignal(sig));
}

static void sighup_handler() {
    sig_hup = 1;
}

// update event on a connection
bool update_event(conn *c, const int new_flags) {
    assert(c != NULL);
    struct event_base *base = c->event.ev_base;
    if (c->event_flags == new_flags) {
        return true;
    }
    if (event_del(&c->event) == -1) {
        return false;
    }
    event_set(&c->event, c->sfd, new_flags, event_handler, (void *)c);
    event_base_set(base, &c->event);
    c->event_flags = new_flags;
    if (event_add(&c->event, 0) == -1) {
        return false;
    }
    return true;
}

void event_handler(const evutil_socket_t fd, const short which, void *arg) {
    conn *c;
    c = (conn *)arg;
    assert(c != NULL);
    c->which = which;
    if (fd != c->sfd) {
        if (settings.verbose > 0) {
            fprintf(stderr, "event fd doesn't match conn fd\n");
        }
        conn_close(c);
        return;
    }
    drive_machine(c);
    return;
}

static void clock_handler(const evutil_socket_t fd, const short which, void *arg) {
    struct timeval t = {.tv_sec = 1, .tv_usec = 0};
    static bool initialized = false;
    if (initialized) {
        // delete the event if it's already exists
        evtimer_del(&clockevent);
    } else {
        initialized = true;
    }
    // check HUP signal
    if (sig_hup) {
        sig_hup = false;
    }
    evtimer_set(&clockevent, clock_handler, 0);
    event_base_set(main_base, &clockevent);
    evtimer_add(&clockevent, &t);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    g_rel_current_time = (rel_time_t) (tv.tv_sec - process_started);
}

// drive state machine
void drive_machine(conn *c) {
    bool stop = false;
    int sfd;
    socklen_t addrlen;
    struct sockaddr_storage addr;
    while (!stop) {
        switch (c->state) {
        case conn_listening:
            addrlen = sizeof(addr);
            sfd = accept(c->sfd, (struct sockaddr *)&addr, &addrlen);
            if (sfd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    stop = true;
                } else if (errno == EMFILE) {
                    if (settings.verbose > 0) {
                        fprintf(stderr, "too many open connections\n");
                    }
                    accept_new_conns(false);
                    stop = true;
                } else {
                    perror("accept()");
                    stop = true;
                }
                break;
            }
            if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK) < 0) {
                perror("setting O_NONBLOCK");
                close(sfd);
                break;
            }
            bool reject = false;
            /*
            if (settings.maxconns_fast) {
                reject = 
            } else {
                reject = false;
            }*/
            if (reject) {

            } else {
                dispatch_conn_new(sfd, conn_new_cmd, EV_READ | EV_PERSIST);
            }
            stop = true;
            break;
        case conn_closing:
            conn_close(c);
            stop = true;
            break;
        case conn_closed:
            abort();
            break;
        }
    }
    return;
}

static void version() {
    printf("in-memory cache service. version %d\n", VERSION);
}

static void usage() {
    version();
    printf("-d, --daemon        run as a daemon\n"
           "-h, --help          print help message and exit\n"
           "-u, --user=<user>   assume identity of <username> (only when run as root)\n"
           "-v  --version       print version message and exit\n"
           );
}
/*
static int server_sockets() {

}
*/

/*
typedef struct test {
    char arr[128];
    int num;
} test_t;

int main(int argc, char **argv) {
    cache_mem_set_default();
    test_t *p = (*settings.m).mem_malloc(sizeof(test_t));
    p->num = 4;
    printf("num = %d\n", p->num);
    (*settings.m).mem_free(p, sizeof(test_t));
}
*/

int main(int argc, char **argv) {
    int do_daemonize = 0;       // daemonize 
    int enable_core = 0;        // generate core dump file 
    char *username = NULL;
    struct passwd *pw;
    struct rlimit rlim;
    int c;  

    int retval = EXIT_SUCCESS;  // return status 

    // handle signals 
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sighup_handler);
    // init settings
    settings_init();
    // process arguments 
    char *shortopts = 
        "d"     // daemon mode 
        "h"     // help 
        "u:"    // user identity to run as 
        "v"     // version 
        ;
#ifdef HAVE_GETOPT_LONG
    const struct option longopts[] = {
        {"daemon", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"user", required_argument, 0, 'u'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    int optindex;
    while ((c = getopt_long(argc, argv, shortopts, longopts, &optindex)) != -1) {
#else
    while ((c = getopt(argc, argv, shortopts)) != -1) {
#endif
        switch (c) {
        case 'd':
            do_daemonize = 1;
            break;
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case 'r':
            enable_core = 1;
            break;
        case 'v':
            version();
            exit(EXIT_SUCCESS);
        case 'u':
            username = optarg;
            break;
        default:
            break;
        }
    }

    if (enable_core != 0) {
        struct rlimit rlim_new;
        /**
         * first try raising to infinity;
         * if fails, try bringing the soft limit to the hard.
         */
        
        if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
            if (setrlimit(RLIMIT_CORE, &rlim_new) != 0) {
                // failed. try raising just to the old max 
                rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
                setlimit(RLIMIT_CORE, &rlim_new);
            }
        }
        /*
         * getrlimit again
         * fail if the soft limit is 0
         */
        
        if ((getrlimit(RLIMIT_CORE, &rlim) != 0) || rlim.rlim_cur == 0) {
            fprintf(stderr, "failed to ensure corefile creation\n");
            exit(EX_OSERR);
        }
    }
    // lose root privileges if we have 
    if (getuid() == 0 || geteuid() == 0) {
        if (username == 0 || *username == '\0') {
            fprintf(stderr, "can't run as root without specify username\n");
            exit(EX_USAGE);
        }
        if ((pw = getpwnam(username)) == 0) {
            fprintf(stderr, "can't find the user %s to switch to\n", username);
            exit(EX_NOUSER);
        }
        if (setgroups(0, NULL) < 0) {
            int should_exit = errno != EPERM;
            fprintf(stderr, "failed to drop supplementary groups: %s\n", strerror(errno));
            if (should_exit) {
                exit(EX_OSERR);
            }
        }
        if (setgid(pw->pw_gid) < 0 || setuid(pw->pw_uid) < 0) {
            fprintf(stderr, "failed to assume identity of user %s\n", username);
            exit(EX_OSERR);
        }
    }
    // daemonize if requested 
    // if we want to ensure our ability to dump core, don't chdir to / 
    if (do_daemonize) {
        // ignore SIGHUP if run as a daemon 
        if (signal(SIGHUP, SIG_IGN)) {
            perror("failed to ignore SIGHUP");
        }
        if (daemonize(enable_core) == -1) {
            fprintf(stderr, "failed to daemonize");
            exit(EXIT_FAILURE);
        }
    }
    // initialize main thread libevent instance 
#if defined(LIBEVENT_VERSION_NUMBER) && LIBEVENT_VERSION_NUMBER >= 0X2000101
    // determine the init api according to libevent library version 
    struct event_config *ev_config;
    ev_config = event_config_new();
    event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);
    main_base = event_base_new_with_config(ev_config);
    event_config_free(ev_config);
#else
    main_base = event_init();
#endif

    process_started = time(0);
    // ignore SIGPIPE signals 
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("failed to ignore SIGPIPE; sigaction");
        exit(EX_OSERR);
    }

    clock_handler(0, 0, 0);

    // enter the event loop 
    while(!stop_main_loop) {
        if (event_base_loop(main_base, EVLOOP_ONCE) != 0) {
            retval = EXIT_FAILURE;
            break;
        }
    }
    switch (stop_main_loop) {
    case EXIT_NORMALLY:
        // normal shutdown 
        break;
    default:
        fprintf(stderr, "exiting on error\n");
        break;
    }

    // cleanup base 
    event_base_free(main_base);

    return retval;
}
