#include <stdio.h>
#include <stdlib.h>
#include "alloc.h"
#include "defs.h"
void* alloc_or_die(size_t nbytes, const char* description) {
    void* data = calloc(nbytes,sizeof(char));
    if (data == NULL) {
        wlog("could not allocate %s\n", description);
        exit(EXIT_FAILURE);
    }
    return data;
}

