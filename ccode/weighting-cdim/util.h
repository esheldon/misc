#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdarg.h>

void pflush(char * format, ...);
FILE* open_or_exit(const char* filename, const char* mode);
size_t count_lines(const char* filename);
//int file_readable(const char* filename);

#endif
