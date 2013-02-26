#include <stdio.h>
#include <stdlib.h>
#include "object.h"

int object_read_one(struct object *self, FILE *fobj)
{
    int nread=0;

    nread=fscanf(fobj,
                 "%s %lf %lf",
                 self->model,
                 &self->row,
                 &self->col);

    if (nread!=3) goto _object_read_one_bail;

    shape_read_e1e2(&self->shape, fobj);
    nread+=2;

    nread += fscanf(fobj,
                    "%lf %lf %s",
                     &self->T,
                     &self->counts,
                     self->psf_model);
    if (nread!=8) goto _object_read_one_bail;

    shape_read_e1e2(&self->psf_shape, fobj);

    nread += 2;

    nread += fscanf(fobj,"%lf", &self->psf_T);


_object_read_one_bail:

    if (nread==EOF) {
        return 0;
    }
    if (nread !=  OBJECT_NFIELDS && nread != 0) {
        fprintf(stderr,"did not read full object\n");
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
                     self->shape.e1,
                     self->shape.e2,
                     self->T,
                     self->counts,

                     self->psf_model,
                     self->psf_shape.e1,
                     self->psf_shape.e2,
                     self->psf_T);

    if (nread != 11) {
        ;
    }
}
