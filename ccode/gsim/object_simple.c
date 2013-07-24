#include <stdio.h>
#include <stdlib.h>
#include "object_simple.h"

int object_simple_read_one(struct object_simple *self, FILE *fobj)
{
    int nread=0;
    double sigma=0; // T = sigma1**2 + sigma2**2

    nread=fscanf(fobj,
                 "%s %lf %lf",
                 self->model,
                 &self->row,
                 &self->col);

    if (nread != 3) goto _object_read_one_bail;

    shape_read_e(&self->shape, fobj);
    nread+=2;

    nread += fscanf(fobj,
                    "%lf %lf %s",
                     &sigma,
                     &self->counts,
                     self->psf_model);
    if (nread!=8) goto _object_read_one_bail;
    self->T = 2*sigma*sigma;

    shape_read_e(&self->psf_shape, fobj);

    nread += 2;

    nread += fscanf(fobj,"%lf", &sigma);
    self->psf_T = 2*sigma*sigma;


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


void object_simple_write_one(struct object_simple *self, FILE* fobj)
{
    int nwrite=fprintf(fobj,
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

    if (nwrite != 11) {
        ;
    }
}
