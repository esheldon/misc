#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gvec.h"
#include "defs.h"

struct gvec* gvec_new(size_t ngauss)
{
    struct gvec*self=NULL;
    if (ngauss == 0) {
        wlog("number of gaussians must be > 0\n");
        return NULL;
    }

    self = calloc(1, sizeof(struct gvec));
    if (self==NULL) {
        wlog("could not allocate struct gvec\n");
        return NULL;
    }

    self->size=ngauss;

    self->data = calloc(self->size, sizeof(struct gauss));
    if (self->data==NULL) {
        wlog("could not allocate %lu gaussian structs\n",ngauss);
        free(self);
        return NULL;
    }

    return self;

}

struct gvec *gvec_free(struct gvec *self)
{
    if (self) {
        free(self->data);
        self->data=NULL;
        free(self);
        self=NULL;
    }
    return self;
}

struct gvec *gvec_copy(struct gvec *self)
{
    struct gvec *copy=gvec_new(self->size);
    if (copy==NULL)
        return NULL;
    memcpy(copy->data, self->data, self->size*sizeof(struct gauss));
    return copy;
}

void gvec_print(FILE* fptr, struct gvec *self)
{
    struct gauss *gptr = self->data;
    for (size_t i=0; i<self->size; i++) {
        wlog("%lu p: %9.6lf row: %9.6lf col: %9.6lf " 
             "irr: %9.6lf irc: %9.6lf icc: %9.6lf\n",
             i, gptr->p, gptr->row, gptr->col,
             gptr->irr,gptr->irc, gptr->icc);
        gptr++;
    }
}
