#ifndef _FILES_H
#define _FILES_H

#include <stdio.h>

#define _COUNTLINES_BUFFSIZE 64

FILE* open_file(const char* fname);
size_t countlines(FILE* fptr);

#endif
