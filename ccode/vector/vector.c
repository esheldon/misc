#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "vector.h"

struct vector* vector_new(size_t num, size_t elsize) {
    struct vector* self = calloc(1,sizeof(struct vector));
    if (self == NULL) {
        fprintf(stderr,"Could not allocate struct vector\n");
        exit(EXIT_FAILURE);
    }

    self->size = 0;
    self->capacity = num;
    self->elsize = elsize;

    if (num > 0) {
        self->d = calloc(num, elsize);
        if (self->d == NULL) {
            free(self);
            fprintf(stderr,
                "Could not allocate %lu elements of size %lu\n",num,elsize);
            exit(EXIT_FAILURE);
        }
    }

    return self;
}

void* vector_front(struct vector* self)
{
    void* p=NULL;
    if (self->size > 0) {
        p = self->d;
    }
    return p;
}
void* vector_back(struct vector* self)
{
    void* p=NULL;
    if (self->size > 0) {
        p = self->d + (self->size-1)*self->elsize;
    }
    return p;
}
