#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "files.h"

FILE* open_file(const char* fname) {
    FILE* fptr=fopen(fname, "r");
    if (fptr==NULL) {
        wlog("Could not open file: %s\n", fname);
        exit(EXIT_FAILURE);
    }
    return fptr;
}


size_t countlines(FILE* fptr) {

    char buff[_COUNTLINES_BUFFSIZE];
    size_t count = 0;
         
    while(fgets(buff,_COUNTLINES_BUFFSIZE,fptr) != NULL) {
        count++;
    }
    return count;
}

