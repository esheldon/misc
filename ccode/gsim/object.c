#include <stdio.h>
#include <stdlib.h>
#include "object.h"

int object_read_one(struct object *self, FILE *fobj)
{
    
    int nread=fscanf(fobj,
                     "%s %lf %lf %lf %lf %lf %lf %s %lf %lf %lf",
                     self->model,
                     &self->row,
                     &self->col,
                     &self->e1,
                     &self->e2,
                     &self->T,
                     &self->counts,

                     self->psf_model,
                     &self->psf_e1,
                     &self->psf_e2,
                     &self->psf_T);

    if (nread==EOF) {
        return 0;
    }
    if (nread !=  11 && nread != 0) {
        fprintf(stderr,"only read %d items from object line\n", nread);
        exit(EXIT_FAILURE);
    }

    return 1;
}


void object_write_one(struct object *self, FILE* fobj)
{
    int nread=fprintf(fobj,
                     "%s %lf %lf %lf %lf %lf %lf %s %lf %lf %lf\n",
                     self->model,
                     self->row,
                     self->col,
                     self->e1,
                     self->e2,
                     self->T,
                     self->counts,

                     self->psf_model,
                     self->psf_e1,
                     self->psf_e2,
                     self->psf_T);

    if (nread != 11) {
        ;
    }
}
