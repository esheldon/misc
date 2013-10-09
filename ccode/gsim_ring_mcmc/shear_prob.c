#include <stdio.h>
#include <stdlib.h>
#include "shear_prob.h"
#include <unistd.h>


struct shear_prob1 *shear_prob1_new(long nshear, double shear_min, double shear_max)
{
    if (nshear <= 1) {
        fprintf(stderr,"nshear must be >= 1, got %ld: %s: %d\n",
                nshear, __FILE__,__LINE__);
        exit(1);
    }


    struct shear_prob1 *self=calloc(1, sizeof(struct shear_prob1));
    if (!self) {
        fprintf(stderr,"could not allocate struct shear_prob1: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    self->nshear = nshear;
    self->shear_min=shear_min;
    self->shear_max=shear_max;

    self->shears = calloc(nshear, sizeof(struct shape));
    if (!self->shears) {
        fprintf(stderr,"could not allocate shears: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }
    self->lnprob = calloc(nshear, sizeof(double));
    if (!self->lnprob) {
        fprintf(stderr,"could not allocate lnprob: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    double step = (shear_max-shear_min)/(nshear-1);

    for (long i=0; i<nshear; i++) {
        struct shape *shear = &self->shears[i];

        double g1 = shear_min + step*i;
        double g2 = 0.0;

        shape_set_g(shear, g1, g2);
    }

    return self;
}

void shear_prob1_write_file(struct shear_prob1 *self, const char *fname)
{
    fprintf(stderr,"writing shear prob file: %s\n", fname);

    FILE *fobj = NULL;
    for (long itry=0; itry<5; itry++) {
        fobj = fopen(fname, "w");
        if (fobj) {
            break;
        }
        fprintf(stderr,"  tried to open but failed...\n");
        sleep(1);
    }
    if (fobj==NULL) {
        fprintf(stderr,"could not open file for writing %s\n", fname);
    }

    shear_prob1_write(self, fobj);
    fclose(fobj);
}
void shear_prob1_write(struct shear_prob1 *self, FILE *fobj)
{
    for (long ishear=0;  ishear<self->nshear; ishear++) {

        struct shape *shear = &self->shears[ishear];
        double lnp = self->lnprob[ishear];
        fprintf(fobj,"%.16g %.16g %.16g\n", 
                shear->g1, shear->g2, lnp);

    }
}
struct shear_prob1 *shear_prob1_free(struct shear_prob1 *self)
{
    if (self) {
        free(self->shears);
        free(self->lnprob);
        free(self);
        self=NULL;
    }
    return self;
}
