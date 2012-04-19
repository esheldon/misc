#ifndef _GMIX_IMAGE_HEADER_GUARD
#define _GMIX_IMAGE_HEADER_GUARD

#include "image.h"
#include "matrix.h"
#include "gvec.h"

#define GMIX_ERROR_NEGATIVE_DET 0x1
#define GMIX_ERROR_MAXIT 0x2
#define GMIX_ERROR_NEGATIVE_DET_FIXCEN 0x4

struct gmix {
    size_t maxiter;
    double tol;
    int fixsky;
    int verbose;
};

int gmix_image(struct gmix* self,
               struct image *image, 
               struct gvec *gvec,
               size_t *niter);

void gmix_set_gvec(struct gvec* gvec, 
                   double* pnew,
                   double* rowsum,
                   double* colsum,
                   double* u2sum,
                   double* uvsum,
                   double* v2sum);
void gmix_set_p_and_cen(struct gvec* gvec, 
                        double* pnew,
                        double* rowsum,
                        double* colsum);
void gmix_set_mean_cen(struct gvec* gvec, struct vec2 *cen_mean);
void gmix_set_p_and_mom(struct gvec* gvec, 
                        double* pnew,
                        double* u2sum,
                        double* uvsum,
                        double* v2sum);

void gmix_set_p(struct gvec* gvec, double* pnew);

/* 
 * in this version we force the centers to coincide.  This requires
 * two separate passes over the pixels, one for getting the new centeroid
 * and then another calculating the covariance matrix using the mean
 * centroid
 */
int gmix_image_fixcen(struct gmix* self,
                      struct image *image, 
                      struct gvec *gvec,
                      size_t *iter);


#endif
