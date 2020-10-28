#include "cache.h"
/* basic logger implementation */
#define LOG_BUF_SIZE 64 * 1024
#define MAX_PATH_SIZE 256
#define MAX_NAME_SIZE 64
#define MAX_BASE_SIZE 128
#define DEFAULT_ROTATE_SIZE 64 * 1024 * 1024    /* 64MB */
#define LINE_MAX 2048
/* time precisions */
#define LOG_TIME_PRECISION_SECOND  's'      /* second */
#define LOG_TIME_PRECISION_MSECOND 'm'      /* millisecond */
#define LOG_TIME_PRECISION_USECOND 'u'      /* microsecond */
#define LOG_TIME_PRECISION_NONE    '0'      /* do not output timestamp */
#define LOGGER_TEST

typedef struct log_level {
    const char *level;
    bool need_sync;
} log_level_t;

log_level_t log_level_map[5] = {
    {"FATAL", true},
    {"ERROR", false},
    {"WARN",  false},
    {"INFO",  true},
    {"DEBUG", true} 
};

typedef struct log_context {
    int log_fd;                             /* log file descriptor, default value is STDERR_FILENO */
    char *log_buf;                          /* log buffer */
    char *current;                          /* current position in buffer */
    pthread_mutex_t log_thread_lock;        /* mutex lock */
    logger_level level;                     /* log level */
    int64_t current_size;                   /* log file current size */
    int64_t rotate_size;                    /* rotate the log file when log file size exceeds this parameter */
    bool rotate_immediately;                /* should rotate immediately */
    char log_base_path[MAX_BASE_SIZE];      /* log base path */
    char log_filename[MAX_NAME_SIZE];       /* save the log filename */
    char help_buf[MAX_PATH_SIZE];           /* helper buffer */
    char rotate_time_format[32];            /* time format for rotate filename, default: %Y%m%d_%H%M%S */
    int fd_flags;                           /* log fd flags */
    char time_precision;                    /* time precision */
    int compress_log_days_before;           /* compress the log files before N days */
    bool log_to_cache;                      /* if write to buffer firstly, then sync to disk, default false */
} log_context_t;

/* shared log context */
static log_context_t shared_context;
/* declarations */
static void log_set_level_inner(log_context_t *context, const logger_level level);
static void log_set_fd_flags_inner(log_context_t *context, const int flags);
static void log_set_rotate_time_format_inner(log_context_t *context, const char *time_format);
static int log_set_filename_inner(log_context_t *context, const char *log_filename);
static void log_set_log_to_cache_inner(log_context_t *context, const bool log_to_cache);
static int log_set_base_path_inner(log_context_t *context, const char *base_path);


/* init shared logger context */
int log_init() {
    if (shared_context.log_buf != NULL) {
        fprintf(stderr, "shared logger context already inited\n");
        return 0;
    }
    return log_do_init(&shared_context);
}

int log_do_init(log_context_t *context) {
    int result;
    memset(context, 0, sizeof(log_context_t));
    /* default logger level */
    context->level = LOGGER_INFO;
    context->log_fd = STDERR_FILENO;
    context->time_precision = LOG_TIME_PRECISION_SECOND;
    context->compress_log_days_before = 1;
    context->rotate_size = DEFAULT_ROTATE_SIZE;
    /* set log base path current work path */
    strcpy(context->log_base_path, ".");
    strcpy(context->log_filename, "simplelog");
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

/* 
 * open a new log file 
 * fullpath: $(base_path)/logs/$(log_filename)
 */
static int log_open(log_context_t *context) {
    char *p = context->help_buf;
    strcpy(context->help_buf, context->log_base_path);
    p += strlen(context->log_base_path);
    strcpy(p, "/logs/");
    p += 6;
    strcpy(p, context->log_filename);
    p += strlen(context->log_filename);
    *p = '\0';
    printf("log file full path: %s\n", context->help_buf);
    if ((context->log_fd = open(context->help_buf, O_WRONLY | O_CREAT | O_APPEND | context->fd_flags, 0644)) < 0) {
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
int log_check_and_mkdir(const char *base_path) {
    char data_path[MAX_BASE_SIZE];
    snprintf(data_path, sizeof(data_path), "%s/logs", base_path);
    fprintf(stderr, "log dir path: %s\n", data_path);
    if (!file_exists(data_path)) {
        if (mkdir(data_path, 0755) != 0) {
            fprintf(stderr, "mkdir \"%s\" fail, errno: %d, error info: %s\n", data_path, errno, STRERROR(errno));
            return -1;
        }
    }
    return 0;
}



/* delete old file */
/*
static int log_delete_old_file(log_context_t *context, const char *old_filename) {
    char full_filename[MAX_PATH_SIZE + 128];
    snprintf(full_filename, sizeof(full_filename), "%s", old_filename);
    if (unlink(full_filename) != 0) {
        if (errno != ENOENT) {
            fprintf(stderr, "unlink fail, errno: %d, error info: %s\n", errno, STRERROR(errno));
        }
        return -1;
    }
    return 0;
}
*/

/* do log rotate */
int log_do_rotate(log_context_t *context) {
    struct tm tm;
    time_t current_time;
    char old_filename[MAX_PATH_SIZE + 32];
    int len;
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
    len = sprintf(old_filename, "%s.", context->help_buf);
    strftime(old_filename + len, sizeof(old_filename) - len, context->rotate_time_format, &tm);
    /* check if old file exists, then replace atomic */
    if (access(old_filename, F_OK) == 0) {
        fprintf(stderr, "file: %s already exist\n", old_filename);
    } else if (rename(context->help_buf, old_filename) != 0) {
        fprintf(stderr, "rename %s to %s fail, errno: %d, error info: %s\n", context->log_filename, old_filename, errno, STRERROR(errno));
    } else {
    }
    result = log_open(context);
    return result;
}

/* check and rotate log */
static int log_check_rotate(log_context_t *context) {
    if (context->log_fd == STDERR_FILENO) {
        if (context->current_size > 0) {
            context->current_size = 0;
        }
        return 0;
    }
    /* if rotate_immediately is set, do log rotate */
    if (context->rotate_immediately) {
        context->rotate_immediately = 0;
        return log_do_rotate(context);
    }
    return 0;
}

/* sync to log file */
static int log_fsync(log_context_t *context, const bool need_lock) {
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
    /* write to log file */
    written = write(context->log_fd, context->log_buf, write_bytes);
    context->current = context->log_buf;
    if (written != write_bytes) {
        result = -1;
        fprintf(stderr, "short write occured, errno: %d, error info: %s\n", errno, STRERROR(errno));
    }
    context->current_size += written;
    if (context->rotate_size > 0) {
        if (context->current_size > context->rotate_size) {
            context->rotate_immediately = 1;
            log_check_rotate(context);
        }
    }
    result = 0;
    if (need_lock) {
        pthread_mutex_unlock(&context->log_thread_lock);
    }
    return result;
    
}

/* do log */
static void log_do_log(log_context_t *context, struct timeval *tv, const char *level_string, const char *text, const int text_len, const bool need_lock, const bool need_sync) {
    int time_fragment;
    struct tm tm;
    int len;
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
    /* if there is not enough buffer space, do fsync */
    if ((context->current - context->log_buf) + text_len + 64 > LOG_BUF_SIZE) {
        log_fsync(context, 0);
    }
    /* format time */
    if (context->time_precision != LOG_TIME_PRECISION_NONE) {
        localtime_r(&tv->tv_sec, &tm);
        if (context->time_precision == LOG_TIME_PRECISION_SECOND) {
            len = sprintf(context->current, "[%04d-%02d-%02d %02d:%02d:%02d] ", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        } else {
            len = sprintf(context->current, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, time_fragment);
        }
        context->current += len;
    }
    /* log level */
    if (level_string != NULL) {
        len = sprintf(context->current, "%s - ", level_string);
        context->current += len;
    }
    memcpy(context->current, text, text_len);
    context->current += text_len;
    *context->current++ = '\n';
    /* fsync if need */
    if (!context->log_to_cache || need_sync) {
        log_fsync(context, false);
    }
    if (need_lock) {
        pthread_mutex_unlock(&context->log_thread_lock);
    }
}

/* wrapper function */
void log_log(log_context_t *context, const char *level_string, const char *text, const int text_len, const bool need_lock, const bool need_sync) {
    struct timeval tv;
    time_t current_time;
    if (context->time_precision == LOG_TIME_PRECISION_SECOND) {
#ifdef LOGGER_TEST
        current_time = time(NULL);
#else
#endif
        tv.tv_sec = current_time;
        tv.tv_usec = 0;
    } else if (context->time_precision != LOG_TIME_PRECISION_NONE) {
        gettimeofday(&tv, NULL);
    }
    log_do_log(context, &tv, level_string, text, text_len, need_lock, need_sync);
}
/* log without format */
void log_log_text(log_context_t *context, const int priority, const char *text, const int text_len) {
    if (context->level < priority) {
        return;
    }
    bool need_sync;
    const char *level_string; 
    level_string = log_level_map[priority].level;
    need_sync = log_level_map[priority].need_sync;
    log_log(context, level_string, text, text_len, true, need_sync);
}

/* log without format: macro form */
#define LOG_TEXT(context, priority, text, text_len) do { \
    if (context->level < priority) { \
        return; \
    } \
    bool need_sync; \
    const char *level_string; \
    level_string = log_level_map[priority].level; \
    need_sync = log_level_map[priority].need_sync; \
    log_log(context, level_string, text, text_len, true, need_sync); \
    } while(0)

/* log with format */
void log_log_textf(log_context_t *context, const int priority, const char *format, ...) {
    if (context->level < priority) {
        return;
    }
    bool need_sync;
    const char *level_string;
    char text[LINE_MAX];
    va_list ap;
    int len;

    va_start(ap, format);
    len = vsnprintf(text, sizeof(text), format, ap);
    va_end(ap);
    if (len >= sizeof(text)) {
        len = sizeof(text) - 1;
    }
    text[len] = '\0';
    level_string = log_level_map[priority].level;
    need_sync = log_level_map[priority].need_sync;
    log_log(context, level_string, text, len, true, need_sync);

}

/* log with format: marco form */
#define LOG_TEXTF(context, priority) do { \
    if (context->level < priority) { \
        return; \
    } \
    bool need_sync; \
    const char *level_string; \
    char text[LINE_MAX]; \
    va_list ap; \
    int len; \
    { \
    va_start(ap, format); \
    len = vsnprintf(text, sizeof(text), format, ap); \
    va_end(ap); \
    if (len >= sizeof(text)) { \
        len = sizeof(text) - 1; \
    } \
    text[len] = '\0'; \
    level_string = log_level_map[priority].level; \
    } \
    need_sync = log_level_map[priority].need_sync; \
    log_log(context, level_string, text, len, true, need_sync); \
    } while(0) 

const char *log_get_level_string(log_context_t *context) {
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



/* exported log functions */
void log_debug(const char *text) {
    LOG_TEXT((&shared_context), LOGGER_DEBUG, text, strlen(text));
}

void log_info(const char *text) {
    LOG_TEXT((&shared_context), LOGGER_INFO, text, strlen(text));
}

void log_warn(const char *text) {
    LOG_TEXT((&shared_context), LOGGER_WARN, text, strlen(text));
}

void log_error(const char *text) {
    LOG_TEXT((&shared_context), LOGGER_ERROR, text, strlen(text));
}

void log_fatal(const char *text) {
    LOG_TEXT((&shared_context), LOGGER_FATAL, text, strlen(text));
}

/* log functions with format */
void log_debugf(const char *format, ...) {
    LOG_TEXTF((&shared_context), LOGGER_DEBUG);
}

void log_infof(const char *format, ...) {
    LOG_TEXTF((&shared_context), LOGGER_INFO);
}

void log_warnf(const char *format, ...) {
    LOG_TEXTF((&shared_context), LOGGER_WARN);
}

void log_errorf(const char *format, ...) {
    LOG_TEXTF((&shared_context), LOGGER_ERROR);
}

void log_fatalf(const char *format, ...) {
    LOG_TEXTF((&shared_context), LOGGER_FATAL);
}

/* exported setters */
void log_set_level(const logger_level level) {
    log_set_level_inner(&shared_context, level);
}

/* only called when log directory is already created */
int log_set_filename(const char *log_filename) {
    return log_set_filename_inner(&shared_context, log_filename);
}

void log_set_log_to_cache(const bool log_to_cache) {
    log_set_log_to_cache_inner(&shared_context, log_to_cache);
}

int log_set_base_path(const char *base_path) {
    return log_set_base_path_inner(&shared_context, base_path);
}

/* internal setters */
static void log_set_level_inner(log_context_t *context, const logger_level level) {
    context->level = level;
}
static void log_set_fd_flags_inner(log_context_t *context, const int flags) {
    context->fd_flags = flags;
}

static void log_set_rotate_time_format_inner(log_context_t *context, const char *time_format) {
    snprintf(context->rotate_time_format, sizeof(context->rotate_time_format), "%s", time_format);
}

static int log_set_filename_inner(log_context_t *context, const char *log_filename) {
    if (log_filename == NULL) {
        fprintf(stderr, "invalid filename\n");
        return -1;
    }
    snprintf(context->log_filename, MAX_NAME_SIZE, "%s", log_filename);
    return log_open(context);
}

static void log_set_log_to_cache_inner(log_context_t *context, const bool log_to_cache) {
    context->log_to_cache = log_to_cache;
}

static int log_set_base_path_inner(log_context_t *context, const char *base_path) {
    int result;
    if ((result = log_check_and_mkdir(base_path)) != 0) {
        return result;
    }
    snprintf(context->log_base_path, MAX_BASE_SIZE, "%s", base_path);
    return log_open(context);
}