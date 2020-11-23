#include "cache.h"

// util functions 


// check if file exists 
bool file_exists(const char *filename) {
    return access(filename, F_OK) == 0;
}
// perror with format 
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

void write_to_1(byte *b, ulint n) {
    assert(n <= 0xff);
    b[0] = (byte)n;
}

ulint read_from_1(byte *b) {
    return ((ulint)(b[0]));
}

void write_to_2(byte *b, ulint n) {
    assert(n <= 0xffff);
    b[0] = (byte)(n >> 8);
    b[1] = (byte)(n);
}

ulint read_from_2(byte *b) {
    return ((ulint)(b[0]) << 8) + (ulint)(b[1]);
}
