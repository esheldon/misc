#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include <stdarg.h>

void pflush(char * format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}



FILE* open_or_exit(const char* filename, const char* mode) {
    FILE* fptr = fopen(filename,mode);
    if (fptr == NULL) {
        fprintf(stderr,"Error opening file '%s': ", filename);
        perror(NULL);
        exit(1);
    }
    return fptr;
}

size_t count_lines(const char* filename) {
    FILE* fptr = open_or_exit(filename,"r");

    char line[256];

    size_t count=0;
    while ( fgets(line, 256, fptr) ) {
        count++;
    }

    fclose(fptr);
    return count;
}

/*
int file_readable(const char* filename) {
    struct stat stFileInfo;
    int retval;
    int status;

    // Attempt to get the file attributes
    status = stat(filename,&stFileInfo);
    if(status == 0) {
        // We were able to get the file attributes
        // so the file exists.
        retval = 1;
    } else {
        // We were not able to get the file attributes.
        // This may mean that we don't have permission to
        // access the folder which contains this file. If you
        // need to do that level of checking, lookup the
        // return values of stat which will give you
        // more details on why stat failed.
        retval = 0;
    }

    return(retval);
}
*/



