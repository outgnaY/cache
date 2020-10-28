#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOGGER_FATAL,
    LOGGER_ERROR,
    LOGGER_WARN,
    LOGGER_INFO,
    LOGGER_DEBUG   
} logger_level;


/* exported log functions */
int log_init();
int log_check_and_mkdir(const char *base_path);
void log_set_level(const logger_level level);
int log_set_filename(const char *log_filename);
void log_set_log_to_cache(const bool log_to_cache);
int log_set_base_path(const char *base_path);

void log_debug(const char *text);
void log_info(const char *text);
void log_warn(const char *text);
void log_error(const char *text);
void log_fatal(const char *text);

void log_debugf(const char *format, ...);
void log_infof(const char *format, ...);
void log_warnf(const char *format, ...);
void log_errorf(const char *format, ...);
void log_fatalf(const char *format, ...);

#endif