#include "cache.h"
/* basic logger implementation */
#define LOG_BUF_SIZE 64 * 1024
#define MAX_PATH_SIZE 256
/* time precisions */
#define LOG_TIME_PRECISION_SECOND  's'      /* second */
#define LOG_TIME_PRECISION_MSECOND 'm'      /* millisecond */
#define LOG_TIME_PRECISION_USECOND 'u'      /* microsecond */
#define LOG_TIME_PRECISION_NONE    '0'      /* do not output timestamp */
#define LOGGER_TEST

typedef struct log_context {
    int log_fd;                         /* log file descriptor, default value is STDERR_FILENO */
    char *log_buf;                     /* log buffer */
    char *current;                      /* current position in buffer */
    pthread_mutex_t log_thread_lock;    /* mutex lock */
    enum log_level level;               /* log level */
    int64_t current_size;               /* log file current size */
    int64_t rotate_size;                /* rotate the log file when log file size exceeds this parameter */
    int rotate_immediately;             /* should rotate immediately */
    char log_filename[MAX_PATH_SIZE];   /* save the log filename */
    char rotate_time_format[32];        /* time format for rotate filename, default: %Y%m%d_%H%M%S */
    int fd_flags;                       /* log fd flags */
    char time_precision;                /* time precision */
    int compress_log_days_before;       /* compress the log files before N days */
} log_context;

/* shared log context */
static log_context s_context;

/* setters */
void log_set_fd_flags(log_context *context, const int flags) {
    context->fd_flags = flags;
}

void log_set_rotate_time_format(log_context *context, const char *time_format) {
    snprintf(context->rotate_time_format, sizeof(context->rotate_time_format), "%s", time_format);
}

/* init shared logger context */
int log_init() {
    if (s_context.log_buf != NULL) {
        fprintf(stderr, "shared logger context already inited\n");
        return 0;
    }
    return do_log_init(&s_context);
}

int do_log_init(log_context *context) {
    int result;
    memset(context, 0, sizeof(context));
    /* default logger level */
    context->level = LOGGER_INFO;
    context->log_fd = STDERR_FILENO;
    context->time_precision = LOG_TIME_PRECISION_SECOND;
    context->compress_log_days_before = 1;
    strcpy(context->rotate_time_format, "%Y%m%d_%H%M%S");
    context->log_buf = (char *)malloc(LOG_BUF_SIZE);
    if (context->log_buf == NULL) {
        fprintf(stderr, "malloc %d bytes fail\n", LOG_BUF_SIZE);
        return -1;
    }
    context->current = context->log_buf;
    if ((result = pthread_mutex_init(&context->log_thread_lock, NULL)) != 0) {
        return result;
    }
    return 0;
}

/* open a new log file */
static int log_open(log_context *context) {
    if ((context->log_fd = open(context->log_filename, O_WRONLY | O_CREAT | O_APPEND | context->fd_flags, 0644)) < 0) {
        fprintf(stderr, "open log file \"%s\" failed, errno: %d, error info: %s\n", context->log_filename, errno, STRERROR(errno));
        context->log_fd = STDERR_FILENO;
        return -1;
    }
    context->current_size = lseek(context->log_fd, 0, SEEK_END);
    if (context->current_size < 0) {
        fprintf(stderr, "lseek file \"%s\" fail, errno: %d, err info: %s\n", context->log_filename, errno, STRERROR(errno));
        return -1;
    } 
    return 0;
}

/* make log directory */
static int check_and_mkdir(const char *base_path) {
    char data_path[MAX_PATH_SIZE];
    snprintf(data_path, sizeof(data_path), "%s/logs", base_path);
    if (!file_exists(data_path)) {
        if (mkdir(data_path, 0755) != 0) {
            fprintf(stderr, "mkdir \"%s\" fail, errno: %d, error info: %s\n", data_path, errno, STRERROR(errno));
            return -1;
        }
    }
    return 0;
}



/* delete old file */
static int log_delete_old_file(log_context *context, const char *old_filename) {
    char full_filename[MAX_PATH_SIZE + 128];
    snprintf(full_filename, sizeof(full_filename), "%s", old_filename);
    /* delete old file */
    if (unlink(full_filename) != 0) {
        if (errno != ENOENT) {
            fprintf(stderr, "unlink fail, errno: %d, error info: %s\n", errno, STRERROR(errno));
        }
        return -1;
    }
    return 0;
}

/* do log rotate */
int do_log_rotate(log_context *context) {
    struct tm tm;
    time_t current_time;
    char old_filename[MAX_PATH_SIZE + 32];
    int len;
    /* int exist = 0; */
    int result;
    /* if we are doing unit test, use time(2) to get current time. otherwise use cached time */
#ifdef LOGGER_TEST
    current_time = time(NULL);
#else
    
#endif
    if (*(context->log_filename) == '\0') {
        return ENOENT;
    }
    /* close current log file */
    close(context->log_fd);
    
    localtime_r(&current_time, &tm);
    memset(old_filename, 0, sizeof(old_filename));
    len = snprintf(old_filename, "%s.", context->log_filename);
    strftime(old_filename + len, sizeof(old_filename) - len, context->rotate_time_format, &tm);
    /* check if old file exists, then replace atomic */
    if (access(old_filename, F_OK) == 0) {
        fprintf(stderr, "file: %s already exist\n", old_filename);
        /* exist = 1; */
    } else if (rename(context->log_filename, old_filename) != 0) {
        fprintf(stderr, "rename %s to %s fail, errno: %d, error info: %s\n", context->log_filename, old_filename, errno, STRERROR(errno));
        /* exist = 0; */
    } else {
        /* exist = 1; */
    }
    result = log_open(context);
    return result;
}

/* check and rotate log */
static int log_check_rotate(log_context *context) {
    if (context->log_fd == STDERR_FILENO) {
        if (context->current_size > 0) {
            context->current_size = 0;
        }
        return 0;
    }
    /* if rotate_immediately is set, do log rotate */
    if (context->rotate_immediately) {
        context->rotate_immediately = 0;
        return do_log_rotate(context);
    }
    return 0;
}

/* sync to file */
static int log_fsync(log_context *context, const int need_lock) {
    int result = 0;
    int write_bytes;
    int written;
    if (context->current - context->log_buf == 0) {
        if (!context->rotate_immediately) {
            return result;
        } else {
            if (need_lock) {
                pthread_mutex_lock(&context->log_thread_lock);
                result = log_check_rotate(context);
                pthread_mutex_unlock(&context->log_thread_lock);
            } else {
                result = log_check_rotate(context);
            }
            return result;
        }
    }
    if (need_lock) {
        pthread_mutex_lock(&context->log_thread_lock);
    }
    write_bytes = context->current - context->log_buf;
    context->current_size += write_bytes;
    if (context->rotate_size > 0) {
        if (context->current_size > context->rotate_size) {
            context->rotate_immediately = 1;
            log_check_rotate(context);
        }
    }
    result = 0;
    /* ??? */
    while ((written = write(context->log_fd, context->log_buf, write_bytes)) < write_bytes) {

    }
    if (context->rotate_immediately) {
        log_check_rotate(context);
    }
    if (need_lock) {
        pthread_mutex_unlock(&context->log_thread_lock);
    }
    return result;
    
}

static void do_log_log(log_context *context, struct timeval *tv, const char *level_string, const char *text, const int text_len, int need_lock) {
    int time_fragment;

    if ((context->time_precision == LOG_TIME_PRECISION_SECOND) || (context->time_precision == LOG_TIME_PRECISION_NONE)) {
        time_fragment = 0;
    } else {
        if (context->time_precision == LOG_TIME_PRECISION_MSECOND) {
            time_fragment = tv->tv_usec / 1000;
        } else {
            time_fragment = tv->tv_usec;
        }
    }
    if (need_lock) {
        pthread_mutex_lock(&context->log_thread_lock);
    }
    if (text_len + 64 > LOG_BUF_SIZE) {
        fprintf(stderr, "too large text: %d\n", text_len + 64);
        if (need_lock) {
            pthread_mutex_unlock(&context->log_thread_lock);
        }
        return;
    }
    if ((context->current - context->log_buf) + text_len + 64 > LOG_BUF_SIZE) {
        log_fsync(context, 0);
    }
}



const char *log_get_level_string(log_context *context) {
    const char *level_string;
    switch (context->level) {
    case LOGGER_DEBUG:
        level_string = "DEBUG";
        break;
    case LOGGER_INFO:
        level_string = "INFO";
        break;
    case LOGGER_WARN:
        level_string = "WARN";
        break;
    case LOGGER_ERROR:
        level_string = "ERROR";
        break;
    case LOGGER_FATAL:
        level_string = "FATAL";
        break;
    default:
        level_string = "UNKNOWN";
        break;
    }
    return level_string;
}



/* log functions */
void log_debug() {

}

void log_info() {

}

void log_warn() {

}

void log_error() {

}

void log_fatal() {

}
