#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gmix.h"
#include "gsim_ring.h"
#include "object.h"

static long read_pars(FILE* stream, double *pars, long npars)
{
    for (long i=0; i<npars; i++) {
        long nread=fscanf(stream, "%lf", &pars[i]);
        if (nread != 1) {
            fprintf(stderr, "error reading object parameter %ld: %s: %d",
                    i, __FILE__,__LINE__);
            return 0;
        }
    }

    return 1;
}

long object_read(struct object* self, FILE* stream)
{
    long nread=0;
    long status=0;
    long flags=0;

    // galaxy model
    nread=fscanf(stream, "%s", self->model_name);
    if (nread != 1) {
        fprintf(stderr, "error reading object model name: %s: %d",
                    __FILE__,__LINE__);
        // need a flag for this
        goto _load_object_bail;
    }

    self->model = gmix_string2model(self->model_name, &flags);
    if (flags != 0) {
        goto _load_object_bail;
    }
    self->npars=ring_get_npars_short(self->model, &flags);
    if (flags != 0) {
        goto _load_object_bail;
    }

    if (!read_pars(stream, self->pars, self->npars)) {
        goto _load_object_bail;
    }

    // psf model
    nread=fscanf(stream, "%s", self->psf_model_name);
    if (nread != 1) {
        fprintf(stderr, "error reading psf model name: %s: %d",
                    __FILE__,__LINE__);
        // need a flag for this
        goto _load_object_bail;
    }


    self->psf_model = gmix_string2model(self->psf_model_name, &flags);
    if (flags != 0) {
        goto _load_object_bail;
    }
    self->psf_npars=ring_get_npars_short(self->psf_model, &flags);
    if (flags != 0) {
        goto _load_object_bail;
    }

    if (!read_pars(stream, self->psf_pars, self->psf_npars)) {
        goto _load_object_bail;
    }

    double shear_eta1=0,shear_eta2=0;
    nread=fscanf(stream,"%lf %lf", &shear_eta1, &shear_eta2);
    if (nread != 2) {
        fprintf(stderr, "error reading shear: %s: %d", __FILE__,__LINE__);
        goto _load_object_bail;
    }

    if (!shape_set_eta(&self->shear, shear_eta1, shear_eta2)) {
        goto _load_object_bail;
    }

    nread=fscanf(stream,"%lf %lf", &self->cen1_offset, &self->cen2_offset);
    if (nread != 2) {
        fprintf(stderr, "error reading cen offset: %s: %d", __FILE__,__LINE__);
        goto _load_object_bail;
    }

    nread=fscanf(stream,"%lf", &self->s2n);
    if (nread != 1) {
        fprintf(stderr, "error reading s2n: %s: %d", __FILE__,__LINE__);
        goto _load_object_bail;
    }

    status=1;

_load_object_bail:
    return status;
}

void object_print(const struct object *self, FILE* stream)
{
    fprintf(stream,"model_name:     %s\n", self->model_name);
    fprintf(stream,"model:          %u\n", self->model);
    fprintf(stream,"npars:          %ld\n", self->npars);
    fprintf(stream,"pars:           ");
    for (long i=0; i<self->npars; i++) {
        fprintf(stream,"%g ", self->pars[i]);
    }
    fprintf(stream,"\n");


    fprintf(stream,"psf_model_name: %s\n", self->psf_model_name);
    fprintf(stream,"model:          %u\n", self->psf_model);
    fprintf(stream,"npars:          %ld\n", self->psf_npars);
    fprintf(stream,"psf_pars:       ");
    for (long i=0; i<self->psf_npars; i++) {
        fprintf(stream,"%g ", self->psf_pars[i]);
    }
    fprintf(stream,"\n");

    fprintf(stream,"shear:          %g %g\n", self->shear.g1, self->shear.g2);
    fprintf(stream,"cen_offset:     %g %g\n", self->cen1_offset, self->cen2_offset);
    fprintf(stream,"s2n:            %g\n", self->s2n);

}
