#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmix.h"
#include "shape.h"
#include "gsim_ring.h"

// For BD, the pars should be length 5
//     [eta,Tbulge,Tdisk,Fbulge,Fdisk]

struct ring_pair *ring_pair_new(enum gmix_model model,
                                double s2n,
                                const double *pars, long npars,
                                const struct gmix *psf, 
                                double shear1, double shear2,
                                long *flags)
{
    long npars_full=8, expected_npars=5;
    double pars1[8] = {0};
    double pars2[8] = {0};
    struct shape1={0}, shape2={0};
    struct ring_pair *self=NULL;

    self=calloc(1, sizeof(struct ring_pair));
    if (self==NULL) {
        fprintf(stderr,"Failed to allocate struct ring_pair: %s: %d\n", 
                __FILE__,__LINE__);
        return NULL;
    }
    self->s2n=s2n;
    self->npars = expected_npars;

    if (npars != expected_npars) {
        fprintf(stderr,"expected npars==%ld but got %ld: %s: %d\n", 
                expected_npars, npars, __FILE__,__LINE__);
        goto _ring_pair_new_bail;
    }
    if (model != GMIX_BD) {
        fprintf(stderr,"only GMIX_BD models supported now, got %d: %s: %d\n", 
                model, __FILE__,__LINE__);
        goto _ring_pair_new_bail;
    }


    double theta1 = 2*M_PI*drand48();
    double theta2 = theta1 + M_PI/2.0;

    double eta1_1 = pars[0]*cos(2*theta1);
    double eta2_1 = pars[0]*sin(2*theta1);

    double eta1_2 = pars[0]*cos(2*theta2);
    double eta2_2 = pars[0]*sin(2*theta2);

    shape_set_eta(&shape1, eta1_1, eta2_1);
    shape_set_eta(&shape2, eta1_2, eta2_2);

    pars1[0] = -1; // arbitrary at this point
    pars1[1] = -1; // arbitrary at this point
    pars1[2] = shape1.g1;
    pars1[3] = shape1.g2;
    pars1[4] = pars[1];
    pars1[5] = pars[2];
    pars1[6] = pars[3];
    pars1[7] = pars[4];

    pars2[0] = -1; // arbitrary at this point
    pars2[1] = -1; // arbitrary at this point
    pars2[2] = shape2.g1;
    pars2[3] = shape2.g2;
    pars2[4] = pars[1];
    pars2[5] = pars[2];
    pars2[6] = pars[3];
    pars2[7] = pars[4];

    self->gmix1=gmix_new_model(model, pars1, npars_full, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }
    self->gmix2=gmix_new_model(model, pars2, npars_full, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }


_ring_pair_new_bail:
    if (*flags != 0) {
        self=ring_pair_free(self);
    }
    return self;

}


struct ring_pair *ring_pair_free(struct ring_pair *self)
{
    if (self) {
        self->gmix1=gmix_free(self->gmix1);
        self->gmix2=gmix_free(self->gmix2);
        free(self);
        self=NULL;
    }
    return self;
}
