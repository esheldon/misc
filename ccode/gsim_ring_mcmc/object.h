#ifndef _CATALOG_HEADER_GUARD
#define _CATALOG_HEADER_GUARD

#include "gmix.h"
#include "shape.h"

// par size is fixed but loaded values vary
struct object {
    // galaxy
    char model_name[20];
    enum gmix_model model;

    long npars;
    double pars[5];

    // PSF
    char psf_model_name[20];
    enum gmix_model psf_model;

    long psf_npars;
    double psf_pars[3];

    double cen1_offset;
    double cen2_offset;

    struct shape shear;

    double s2n;
};

// return is 1 for success, 0 for failure
long object_read(struct object *self, FILE* stream);

void object_print(const struct object *self, FILE* stream);


#endif
