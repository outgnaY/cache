#include "common_define.h"

bool file_exists(const char *filename);

void vperror(const char *fmt, ...);

void write_to_1(
    byte *b,        // pointer to byte where to store
    ulint n);       // integer to be stored

ulint read_from_1(
    byte *b);       // pointer to byte to be read

void write_to_2(
    byte *b,        // pointer to 2 bytes to store
    ulint n);       // integer to be stored

ulint read_from_2(
    byte *b);       // pointer to 2 bytes to read

