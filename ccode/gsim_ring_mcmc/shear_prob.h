#ifndef _SHEAR_PROB_HEADER_GUARD
#define _SHEAR_PROB_HEADER_GUARD

#include "shape.h"

// shear in only one component
struct shear_prob1 {

    long nshear;
    double shear_min;
    double shear_max;
    struct shape *shears;
    double *lnprob;
};

struct shear_prob1 *shear_prob1_new(long nshear, double shear_min, double shear_max);
struct shear_prob1 *shear_prob1_free(struct shear_prob1 *self);
void shear_prob1_write(struct shear_prob1 *self, FILE *fobj);
void shear_prob1_write_file(struct shear_prob1 *self, const char *fname);

#endif
