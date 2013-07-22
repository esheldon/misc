#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmix.h"
#include "image_rand.h"
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

// centers for all models are set to 0, which means the convolutions
// will not case problems.  If you want to use non-cocentric psfs you
// will need to be careful!



static void fill_pars_6par(const double *inpars,
                           const struct shape *shape1,
                           const struct shape *shape2,
                           double *pars1,
                           double *pars2)
{
    pars1[0] = 0; // arbitrary at this point
    pars1[1] = 0; // arbitrary at this point
    pars1[2] = shape1->g1;
    pars1[3] = shape1->g2;
    pars1[4] = inpars[1];
    pars1[5] = inpars[2];

    pars2[0] = 0; // arbitrary at this point
    pars2[1] = 0; // arbitrary at this point
    pars2[2] = shape2->g1;
    pars2[3] = shape2->g2;
    pars2[4] = inpars[1];
    pars2[5] = inpars[2];
}
static void fill_pars_6par_psf(const double *inpars, double *pars)
{
    struct shape shape;
    shape_set_eta(&shape, inpars[0], inpars[1]);
    pars[0] = 0; // arbitrary at this point
    pars[1] = 0; // arbitrary at this point
    pars[2] = shape.g1;
    pars[3] = shape.g2;
    pars[4] = inpars[2];
    pars[5] = 1; // arbitrary
}

static void fill_pars_bd(const double *inpars,
                         const struct shape *shape1,
                         const struct shape *shape2,
                         double *pars1,
                         double *pars2)
{
    pars1[0] = 0; // arbitrary at this point
    pars1[1] = 0; // arbitrary at this point
    pars1[2] = shape1->g1;
    pars1[3] = shape1->g2;
    pars1[4] = inpars[1];
    pars1[5] = inpars[2];
    pars1[6] = inpars[3];
    pars1[7] = inpars[4];

    pars2[0] = 0; // arbitrary at this point
    pars2[1] = 0; // arbitrary at this point
    pars2[2] = shape2->g1;
    pars2[3] = shape2->g2;
    pars2[4] = inpars[1];
    pars2[5] = inpars[2];
    pars2[6] = inpars[3];
    pars2[7] = inpars[4];
}

static long check_npars(enum gmix_model model, long npars)
{
    long status=0;
    long flags=0;
    long expected_npars = ring_get_npars_short(model, &flags);
    if (flags == 0) {
        if (npars != expected_npars) {
            fprintf(stderr,"expected npars==%ld but got %ld: %s: %d\n", 
                    expected_npars, npars, __FILE__,__LINE__);
        } else {
            status=1;
        }
    }

    return status;
}

// for simple, pars are
//     [eta,T,F]
//
// For BD, the pars should be length 5
//     [eta,Tbulge,Tdisk,Fbulge,Fdisk]

struct ring_pair *ring_pair_new(enum gmix_model model,
                                const double *pars, long npars,
                                enum gmix_model psf_model,
                                const double *psf_pars,
                                long psf_npars,
                                const struct shape *shear,
                                double s2n,
                                double cen1_offset,
                                double cen2_offset,
                                long *flags)
{
    double pars1[8] = {0};
    double pars2[8] = {0};
    struct shape shape1={0}, shape2={0};
    struct ring_pair *self=NULL;
    struct gmix *gmix1_0=NULL, *gmix2_0=NULL, *psf_gmix=NULL;
    long npars_full=0, psf_npars_full=0;

    if (!check_npars(model, npars) || !check_npars(psf_model,psf_npars)) {
        goto _ring_pair_new_bail;
    }

    self=calloc(1, sizeof(struct ring_pair));
    if (self==NULL) {
        fprintf(stderr,"Failed to allocate struct ring_pair: %s: %d\n", 
                __FILE__,__LINE__);
        return NULL;
    }
    self->s2n=s2n;
    self->cen1_offset=cen1_offset;
    self->cen2_offset=cen2_offset;

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
        npars_full = 8;
        fill_pars_bd(pars, &shape1, &shape2, pars1, pars2);
    } else {
        npars_full = 6;
        fill_pars_6par(pars, &shape1, &shape2, pars1, pars2);
    }

    psf_npars_full = 6;
    double psf_pars_full[6]={0};
    fill_pars_6par_psf(psf_pars, psf_pars_full);

    psf_gmix = gmix_new_model(psf_model, psf_pars_full, psf_npars_full, flags);

    gmix1_0=gmix_new_model(model, pars1, npars_full, flags);
    gmix2_0=gmix_new_model(model, pars2, npars_full, flags);
    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }

    self->gmix1 = gmix_convolve(gmix1_0, psf_gmix, flags);
    self->gmix2 = gmix_convolve(gmix2_0, psf_gmix, flags);
    self->psf_gmix = psf_gmix;

    //fprintf(stderr,"psf_T: %g obj1_T: %g obj2_T: %g\n",
    //        gmix_get_T(self->psf_gmix),
    //        gmix_get_T(self->gmix1),
    //        gmix_get_T(self->gmix2));
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
        self->psf_gmix=gmix_free(self->psf_gmix);
        free(self);
        self=NULL;
    }
    return self;
}

void ring_pair_print(const struct ring_pair *self, FILE* stream)
{
    fprintf(stream,"s2n:   %g\n", self->s2n);

    fprintf(stream,"gmix1:\n");
    gmix_print(self->gmix1, stream);
    fprintf(stream,"gmix2:\n");
    gmix_print(self->gmix2, stream);
}


static struct image *make_image(const struct gmix *gmix,
                                int nsub,
                                double s2n,
                                double cen1_offset,
                                double cen2_offset,
                                double *cen1,
                                double *cen2,
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

    *cen1 =( ((float)box_size) - 1.0)/2.0;
    *cen2 =*cen1;

    *cen1 += cen1_offset;
    *cen2 += cen2_offset;

    tmp_gmix = gmix_new_copy(gmix, flags);
    if (*flags != 0) {
        goto _ring_make_image_bail;
    }
    gmix_set_cen(tmp_gmix, *cen1, *cen2);

    image = gmix_image_new(tmp_gmix, box_size, box_size, nsub);
    if (!image) {
        goto _ring_make_image_bail;
    }

    image_add_randn_matched(image, s2n, skysig);

_ring_make_image_bail:
    tmp_gmix=gmix_free(tmp_gmix);
    if (*flags != 0) {
        image=image_free(image);
    }

    return image;
}

struct ring_image_pair *ring_image_pair_new(const struct ring_pair *self, long *flags)
{
    struct ring_image_pair *impair=NULL;

    impair = calloc(1, sizeof(struct ring_image_pair));
    if (!impair) {
        fprintf(stderr, "could not allocate struct ring_image_pair: %s: %d",
                 __FILE__,__LINE__);
        exit(1);
    }

    impair->cen1_offset=self->cen1_offset;
    impair->cen2_offset=self->cen2_offset;

    impair->im1 = make_image(self->gmix1,
                             RING_IMAGE_NSUB,
                             self->s2n,
                             self->cen1_offset,
                             self->cen2_offset,
                             &impair->cen1,
                             &impair->cen2,
                             &impair->skysig1,
                             flags);
    if (*flags != 0) {
        goto _ring_make_image_pair_bail;
    }
    impair->im2 = make_image(self->gmix2,
                             RING_IMAGE_NSUB,
                             self->s2n,
                             self->cen1_offset,
                             self->cen2_offset,
                             &impair->cen1,
                             &impair->cen2,
                             &impair->skysig2,
                             flags);
    if (*flags != 0) {
        goto _ring_make_image_pair_bail;
    }

    impair->psf_image = make_image(self->psf_gmix,
                                   RING_IMAGE_NSUB,
                                   RING_PSF_S2N,
                                   self->cen1_offset,
                                   self->cen2_offset,
                                   &impair->psf_cen1,
                                   &impair->psf_cen2,
                                   &impair->psf_skysig,
                                   flags);
    if (*flags != 0) {
        goto _ring_make_image_pair_bail;
    }

    double ivar1=1/(impair->skysig1*impair->skysig1);
    double ivar2=1/(impair->skysig2*impair->skysig2);

    impair->wt1 = image_new(IM_NROWS(impair->im1),IM_NCOLS(impair->im1));
    impair->wt2 = image_new(IM_NROWS(impair->im2),IM_NCOLS(impair->im2));
    image_add_scalar(impair->wt1, ivar1);
    image_add_scalar(impair->wt2, ivar2);

_ring_make_image_pair_bail:
    if (*flags != 0) {
        impair=ring_image_pair_free(impair);
    }

    return impair;
}

struct ring_image_pair *ring_image_pair_free(struct ring_image_pair *self)
{
    if (self) {
        self->im1=image_free(self->im1);
        self->im2=image_free(self->im2);
        self->wt1=image_free(self->wt1);
        self->wt2=image_free(self->wt2);
        self->psf_image=image_free(self->psf_image);
        free(self);
        self=NULL;
    }
    return self;
}
