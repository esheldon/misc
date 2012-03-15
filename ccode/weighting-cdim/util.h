#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdarg.h>

#define wlog(...) fprintf(stderr, __VA_ARGS__)

void pflush(char * format, ...);
FILE* open_or_exit(const char* filename, const char* mode);
size_t count_lines(const char* filename);
//int file_readable(const char* filename);

#endif
