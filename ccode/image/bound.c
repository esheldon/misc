#include <stdlib.h>
#include <stdio.h>
#include "bound.h"

struct bound *bound_new(ssize_t rowmin, 
                        ssize_t rowmax, 
                        ssize_t colmin, 
                        ssize_t colmax)
{
    struct bound *self=NULL;

    self = calloc(1, sizeof(struct bound));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct bound\n");
        exit(EXIT_FAILURE);
    }

    bound_set(self, rowmin, rowmax, colmin, colmax);
    return self;
}

struct bound *bound_free(struct bound *self) {
    if (self) {
        free(self);
    }
    return NULL;
}

void bound_set(struct bound* self,
               ssize_t rowmin, 
               ssize_t rowmax, 
               ssize_t colmin, 
               ssize_t colmax)
{
    self->rowmin=rowmin;
    self->rowmax=rowmax;
    self->colmin=colmin;
    self->colmax=colmax;
}

void bound_print(const struct bound *bound, FILE *stream)
{
    fprintf(stream,"  rowmin: %ld\n", bound->rowmin);
    fprintf(stream,"  rowmax: %ld\n", bound->rowmax);
    fprintf(stream,"  colmin: %ld\n", bound->colmin);
    fprintf(stream,"  colmin: %ld\n", bound->colmax);
}
