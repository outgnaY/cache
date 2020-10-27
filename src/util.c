#include "cache.h"

/* util functions */

/* perror with format */
void vperror(const char *fmt, ...) {
    int old_errno = errno;
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    if (vsnprintf(buf, sizeof(buf), fmt, ap) == -1) {
        buf[sizeof(buf) - 1] = '\0';
    }
    va_end(ap);
    errno = old_errno;
    perror(buf);
}
