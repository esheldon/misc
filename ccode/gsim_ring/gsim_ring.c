#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gmix.h"
#include "image_rand.h"
#include "gmix_image.h"
#include "gmix_image_rand.h"
#include "shape.h"
#include "dist.h"
#include "gsim_ring.h"

void fill_shape_prior(struct gsim_ring *self)
{

    switch (self->conf.shape_prior) {

        case DIST_GMIX3_ETA:
            {
                struct dist_gmix3_eta *shape_prior=
                    dist_gmix3_eta_new(self->conf.shape_prior_pars[0],
                                       self->conf.shape_prior_pars[1],
                                       self->conf.shape_prior_pars[2],
                                       self->conf.shape_prior_pars[3],
                                       self->conf.shape_prior_pars[4],
                                       self->conf.shape_prior_pars[5]);

                self->shape_prior = (void *) shape_prior;
            }
            break;
        case DIST_G_BA:
            {
                struct dist_g_ba *shape_prior=dist_g_ba_new(self->conf.shape_prior_pars[0]);
                self->shape_prior = (void *) shape_prior;
            }
            break;
        default:
            fprintf(stderr, "bad shape prior type: %u: %s: %d, aborting\n",
                    self->conf.shape_prior, __FILE__,__LINE__);
            exit(1);
    }
}
struct gsim_ring *gsim_ring_new_from_config(const struct gsim_ring_config *conf)
{
    struct gsim_ring *self=calloc(1,sizeof(struct gsim_ring));
    if (!self) {
        fprintf(stderr, "Could not allocate struct gsim_ring: %s: %d",
                __FILE__,__LINE__);
        exit(1);
    }
    // a value type
    self->conf = (*conf);

    dist_gauss_fill(&self->cen1_dist, conf->cen_prior_pars[0], conf->cen_prior_pars[1]);
    dist_gauss_fill(&self->cen2_dist, conf->cen_prior_pars[0], conf->cen_prior_pars[1]);

    fill_shape_prior(self);

    dist_lognorm_fill(&self->T_dist, conf->T_prior_pars[0], conf->T_prior_pars[1]);
    dist_lognorm_fill(&self->counts_dist, conf->counts_prior_pars[0], conf->counts_prior_pars[1]);

    return self;
}

struct gsim_ring *gsim_ring_new_from_file(const char *name, long *flags)
{
    struct gsim_ring *self=NULL;
    struct gsim_ring_config config={{0}};

    *flags = gsim_ring_config_load(&config, name);
    if (*flags == 0) {
        self=gsim_ring_new_from_config(&config);
    }

    return self;
}

struct gsim_ring *gsim_ring_free(struct gsim_ring *self)
{
    if (self) {
        free(self->shape_prior);
        free(self);
        self=NULL;
    }
    return self;
}

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

static void fill_pars_6par(const struct shape *shape1,
                           const struct shape *shape2,
                           double T, double counts,
                           double *pars1,
                           double *pars2)
{
    pars1[0] = 0;
    pars1[1] = 0;
    pars1[2] = shape1->eta1;
    pars1[3] = shape1->eta2;
    pars1[4] = T;
    pars1[5] = counts;

    pars2[0] = 0;
    pars2[1] = 0;
    pars2[2] = shape2->eta1;
    pars2[3] = shape2->eta2;
    pars2[4] = T;
    pars2[5] = counts;
}


static void fill_pars_6par_psf(const struct shape *shape,
                               double T,
                               double *pars)
{
    pars[0] = 0;
    pars[1] = 0;
    pars[2] = shape->eta1;
    pars[3] = shape->eta2;
    pars[4] = T;
    pars[5] = 1; // arbitrary
}


static void sample_shape_prior(const struct gsim_ring *self, struct shape *shape)
{
    switch (self->conf.shape_prior) {

        case DIST_GMIX3_ETA:
            {
                const struct dist_gmix3_eta *shape_prior=
                    (struct dist_gmix3_eta *) self->shape_prior;
                dist_gmix3_eta_sample(shape_prior, shape);
            }
            break;
        case DIST_G_BA:
            {
                const struct dist_g_ba *shape_prior=
                    (struct dist_g_ba *) self->shape_prior;
                dist_g_ba_sample(shape_prior, shape);
            }
            break;
        default:
            fprintf(stderr, "bad shape prior type: %u: %s: %d, aborting\n",
                    self->conf.shape_prior, __FILE__,__LINE__);
            exit(1);
    }

}

struct ring_pair *ring_pair_new(const struct gsim_ring *ring,
                                double cen1_offset, double cen2_offset,
                                double T, double counts,
                                const struct shape *shape1_in,
                                const struct shape *shape2_in,
                                long *flags)
{

    double pars1[6] = {0};
    double pars2[6] = {0};
    double psf_pars[6]={0};

    struct ring_pair *self=NULL;
    struct gmix *gmix1_0=NULL, *gmix2_0=NULL, *psf_gmix=NULL;
    long npars=6, psf_npars=6;

    self=calloc(1, sizeof(struct ring_pair));
    if (self==NULL) {
        fprintf(stderr,"Failed to allocate struct ring_pair: %s: %d\n", 
                __FILE__,__LINE__);
        return NULL;
    }
    self->psf_s2n=ring->conf.psf_s2n;

    self->cen1_offset = cen1_offset;
    self->cen2_offset = cen2_offset;

    struct shape shape1=(*shape1_in);
    struct shape shape2=(*shape2_in);

    if (!shape_add_inplace(&shape1, &ring->conf.shear)) {
        *flags |= SHAPE_RANGE_ERROR;
        goto _ring_pair_new_bail;
    }
    if (!shape_add_inplace(&shape2, &ring->conf.shear)) {
        *flags |= SHAPE_RANGE_ERROR;
        goto _ring_pair_new_bail;
    }

    // pars gets filled with eta
    fill_pars_6par(&shape1, &shape2, T, counts, pars1, pars2);
    fill_pars_6par_psf(&ring->conf.psf_shape, ring->conf.psf_T, psf_pars);

    // eta
    psf_gmix = gmix_new_model_from_array(ring->conf.psf_model,
                                         psf_pars, psf_npars,
                                         SHAPE_SYSTEM_ETA, flags);
    gmix1_0=gmix_new_model_from_array(ring->conf.obj_model,
                                      pars1, npars,
                                      SHAPE_SYSTEM_ETA, flags);
    gmix2_0=gmix_new_model_from_array(ring->conf.obj_model,
                                      pars2, npars,
                                      SHAPE_SYSTEM_ETA, flags);

    if (*flags != 0) {
        goto _ring_pair_new_bail;
    }

    self->gmix1 = gmix_convolve(gmix1_0, psf_gmix, flags);
    self->gmix2 = gmix_convolve(gmix2_0, psf_gmix, flags);
    self->psf_gmix = psf_gmix;

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


struct ring_pair *ring_pair_new_sample(const struct gsim_ring *ring,
                                       long *flags)
{

    struct shape shape1={0}, shape2={0};

    double cen1_offset = dist_gauss_sample(&ring->cen1_dist);
    double cen2_offset = dist_gauss_sample(&ring->cen2_dist);

    double T = dist_lognorm_sample(&ring->T_dist);
    double counts = dist_lognorm_sample(&ring->counts_dist);

    sample_shape_prior(ring, &shape1);

    shape2 = shape1;
    shape_rotate(&shape2, M_PI_2);

    return ring_pair_new(ring, cen1_offset, cen2_offset, T, counts,
                         &shape1, &shape2, flags);
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

    fprintf(stream,"gmix1:\n");
    gmix_print(self->gmix1, stream);
    fprintf(stream,"gmix2:\n");
    gmix_print(self->gmix2, stream);
}


static struct image *make_image(const struct gmix *gmix,
                                int nsub,
                                double cen1_offset,
                                double cen2_offset,
                                double *coord_cen1,
                                double *coord_cen2,
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

    *coord_cen1 =( ((float)box_size) - 1.0)/2.0;
    *coord_cen2 =*coord_cen1;

    double cen1 = *coord_cen1 + cen1_offset;
    double cen2 = *coord_cen2 + cen2_offset;

    tmp_gmix = gmix_new_copy(gmix, flags);
    if (*flags != 0) {
        goto _ring_make_image_bail;
    }
    gmix_set_cen(tmp_gmix, cen1, cen2);

    image = gmix_image_new(tmp_gmix, box_size, box_size, nsub);
    if (!image) {
        goto _ring_make_image_bail;
    }


_ring_make_image_bail:
    tmp_gmix=gmix_free(tmp_gmix);
    if (*flags != 0) {
        image=image_free(image);
    }

    return image;
}

struct ring_image_pair *ring_image_pair_new(const struct ring_pair *self,
                                            double skysig,
                                            long *flags)
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
                             self->cen1_offset,
                             self->cen2_offset,
                             &impair->coord_cen1,
                             &impair->coord_cen2,
                             flags);
    if (*flags != 0) {
        goto _ring_make_image_pair_bail;
    }
    impair->im2 = make_image(self->gmix2,
                             RING_IMAGE_NSUB,
                             self->cen1_offset,
                             self->cen2_offset,
                             &impair->coord_cen1,
                             &impair->coord_cen2,
                             flags);
    if (*flags != 0) {
        goto _ring_make_image_pair_bail;
    }

    impair->psf_image = make_image(self->psf_gmix,
                                   RING_IMAGE_NSUB,
                                   self->cen1_offset,
                                   self->cen2_offset,
                                   &impair->psf_coord_cen1,
                                   &impair->psf_coord_cen2,
                                   flags);
    if (*flags != 0) {
        goto _ring_make_image_pair_bail;
    }

    impair->skysig1=skysig;
    impair->skysig2=skysig;

    image_add_randn(impair->im1, skysig);
    image_add_randn(impair->im2, skysig);
    image_add_randn_matched(impair->psf_image,
                            self->psf_s2n,
                            &impair->psf_skysig);

    double ivar=1/(skysig*skysig);

    impair->wt1 = image_new(IM_NROWS(impair->im1),IM_NCOLS(impair->im1));
    impair->wt2 = image_new(IM_NROWS(impair->im2),IM_NCOLS(impair->im2));
    image_add_scalar(impair->wt1, ivar);
    image_add_scalar(impair->wt2, ivar);

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
