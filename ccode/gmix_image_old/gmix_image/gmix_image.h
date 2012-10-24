#ifndef _GMIX_IMAGE_HEADER_GUARD
#define _GMIX_IMAGE_HEADER_GUARD

#include "image.h"
#include "gvec.h"

#define GMIX_ERROR_NEGATIVE_DET 0x1
#define GMIX_ERROR_MAXIT 0x2
#define GMIX_ERROR_NEGATIVE_DET_SAMECEN 0x4

struct gmix {
    size_t maxiter;
    double tol;
    int fixsky;
    int verbose;
};

struct sums {
    // scratch on a given pixel
    double gi;
    double trowsum;
    double tcolsum;
    double tu2sum;
    double tuvsum;
    double tv2sum;

    // sums over all pixels
    double pnew;
    double rowsum;
    double colsum;
    double u2sum;
    double uvsum;
    double v2sum;
};

struct iter {
    size_t ngauss;

    // sums over all pixels and all gaussians
    double gtot;
    double skysum;
    double psum;

    double psky;
    double nsky;

    struct sums *sums;
};

int gmix_image(struct gmix* self,
               struct image *image, 
               struct gvec *gvec,
               size_t *niter);
/*
int gmix_image_old(struct gmix* self,
               struct image *image, 
               struct gvec *gvec,
               size_t *niter);
*/


int gmix_get_sums(struct image *image,
                  struct gvec *gvec,
                  struct iter* iter_struct);



struct iter *iter_new(size_t ngauss);
struct iter *iter_free(struct iter *self);
void iter_clear(struct iter *self);

void gmix_set_gvec_fromiter(struct gvec* gvec, struct iter* iter);


#endif
