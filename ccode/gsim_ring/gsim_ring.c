#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmix.h"
#include "gmix_image.h"
#include "gmix_image_rand.h"
#include "shape.h"
#include "gsim_ring.h"

long ring_get_npars_short(enum gmix_model model, long *flags)
{
    long npars=-1;
    switch (model) {
        case GMIX_EXP:
            npars=3;
            break;
        case GMIX_DEV:
            npars=3;
            break;
        case GMIX_BD:
            npars=5;
            break;
        case GMIX_COELLIP: // for psf
            npars=3;
            break;
        case GMIX_TURB: // for psf
            npars=3;
            break;
        default:
            fprintf(stderr, "bad model type: %u: %s: %d",
                    model, __FILE__,__LINE__);
            *flags |= GMIX_BAD_MODEL;
            break;
    }
    return npars;
}

static void fill_pars_6par(const double *inpars,
                           const struct shape *shape1,
                           const struct shape *shape2,
                           double *pars1,
                           double *pars2)
{
    pars1[0] = -1; // arbitrary at this point
    pars1[1] = -1; // arbitrary at this point
    pars1[2] = shape1->g1;
    pars1[3] = shape1->g2;
    pars1[4] = inpars[1];
    pars1[5] = inpars[2];

    pars2[0] = -1; // arbitrary at this point
    pars2[1] = -1; // arbitrary at this point
    pars2[2] = shape2->g1;
    pars2[3] = shape2->g2;
    pars2[4] = inpars[1];
    pars2[5] = inpars[2];
}
static void fill_pars_bd(const double *inpars,
                         const struct shape *shape1,
                         const struct shape *shape2,
                         double *pars1,
                         double *pars2)
{
    pars1[0] = -1; // arbitrary at this point
    pars1[1] = -1; // arbitrary at this point
    pars1[2] = shape1->g1;
    pars1[3] = shape1->g2;
    pars1[4] = inpars[1];
    pars1[5] = inpars[2];
    pars1[6] = inpars[3];
    pars1[7] = inpars[4];

    pars2[0] = -1; // arbitrary at this point
    pars2[1] = -1; // arbitrary at this point
    pars2[2] = shape2->g1;
    pars2[3] = shape2->g2;
    pars2[4] = inpars[1];
    pars2[5] = inpars[2];
    pars2[6] = inpars[3];
    pars2[7] = inpars[4];
}

// for simple, pars are
//     [eta,T,F]
//
// For BD, the pars should be length 5
//     [eta,Tbulge,Tdisk,Fbulge,Fdisk]

struct ring_pair *ring_pair_new(enum gmix_model model,
                                const double *pars, long npars,
                                const struct gmix *psf, 
                                const struct shape *shear,
                                double s2n,
                                long *flags)
{
    double pars1[8] = {0};
    double pars2[8] = {0};
    struct shape shape1={0}, shape2={0};
    struct ring_pair *self=NULL;
    struct gmix *gmix1_0=NULL, *gmix2_0=NULL;

    long expected_npars = ring_get_npars_short(model, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }
    if (npars != expected_npars) {
        fprintf(stderr,"expected npars==%ld but got %ld: %s: %d\n", 
                expected_npars, npars, __FILE__,__LINE__);
        goto _ring_pair_new_bail;
    }

    self=calloc(1, sizeof(struct ring_pair));
    if (self==NULL) {
        fprintf(stderr,"Failed to allocate struct ring_pair: %s: %d\n", 
                __FILE__,__LINE__);
        return NULL;
    }
    self->s2n=s2n;

    double theta1 = 2*M_PI*drand48();
    double theta2 = theta1 + M_PI/2.0;

    double eta1_1 = pars[0]*cos(2*theta1);
    double eta2_1 = pars[0]*sin(2*theta1);

    double eta1_2 = pars[0]*cos(2*theta2);
    double eta2_2 = pars[0]*sin(2*theta2);

    shape_set_eta(&shape1, eta1_1, eta2_1);
    shape_set_eta(&shape2, eta1_2, eta2_2);
    shape_add_inplace(&shape1, shear);
    shape_add_inplace(&shape2, shear);

    if (model==GMIX_BD) {
        self->npars = 8;
        fill_pars_bd(pars,
                     &shape1,
                     &shape2,
                     pars1,
                     pars2);
    } else {
        if (npars != 3) {
            fprintf(stderr,"expected npars==%d but got %ld: %s: %d\n", 
                    3, npars, __FILE__,__LINE__);
            goto _ring_pair_new_bail;
        }

        self->npars = 6;
        fill_pars_6par(pars,
                       &shape1,
                       &shape2,
                       pars1,
                       pars2);
    }

    gmix1_0=gmix_new_model(model, pars1, self->npars, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }
    gmix2_0=gmix_new_model(model, pars2, self->npars, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }

    self->gmix1 = gmix_convolve(gmix1_0, psf, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }
    self->gmix2 = gmix_convolve(gmix2_0, psf, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }


_ring_pair_new_bail:
    if (*flags != 0) {
        self=ring_pair_free(self);
    }
    gmix1_0 = gmix_free(gmix1_0);
    gmix2_0 = gmix_free(gmix2_0);
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

void ring_pair_print(const struct ring_pair *self, FILE* stream)
{
    fprintf(stream,"s2n:   %g\n", self->s2n);
    fprintf(stream,"npars: %ld\n", self->npars);

    fprintf(stream,"gmix1:\n");
    gmix_print(self->gmix1, stream);
    fprintf(stream,"gmix2:\n");
    gmix_print(self->gmix2, stream);
}


struct image *ring_make_image(const struct gmix *gmix,
                              double cen1_offset,
                              double cen2_offset,
                              int nsub,
                              double s2n,
                              double *skysig, // output
                              long *flags) // output
{
    struct gmix *tmp_gmix=NULL;
    struct image *image=NULL;

    // only really works for cocentered, but close enough
    double T = gmix_get_T(gmix);

    double sigma = T > 0 ? sqrt(T/2) : 2;

    long box_size = (long) ( 2*sigma*GAUSS_PADDING );
    if ((box_size % 2) == 0) {
        box_size+=1;
    }

    double cen=( ((float)box_size) - 1.0)/2.0;

    tmp_gmix = gmix_new_copy(gmix, flags);
    if (*flags != 0) {
        goto _ring_make_image_bail;
    }
    gmix_set_cen(tmp_gmix, cen+cen1_offset, cen+cen2_offset);

    image = gmix_image_new(tmp_gmix, box_size, box_size, nsub);
    if (!image) {
        goto _ring_make_image_bail;
    }

    (*flags) |= gmix_image_add_randn(image,
                                     s2n,
                                     tmp_gmix,
                                     skysig);
    if (*flags != 0) {
        goto _ring_make_image_bail;
    }

_ring_make_image_bail:
    tmp_gmix=gmix_free(tmp_gmix);
    if (*flags != 0) {
        image=image_free(image);
    }

    return image;
}
