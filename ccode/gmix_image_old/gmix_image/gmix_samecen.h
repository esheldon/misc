#ifndef _GMIX_IMAGE_SAMECEN_HEADER_GUARD
#define _GMIX_IMAGE_SAMECEN_HEADER_GUARD

#include "gmix_image.h"
#include "image.h"
#include "matrix.h"
#include "gvec.h"

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


/* 
 * in this version we force the centers to coincide.  This requires
 * two separate passes over the pixels, one for getting the new centeroid
 * and then another calculating the covariance matrix using the mean
 * centroid
 */
int gmix_image_samecen(struct gmix* self,
                       struct image *image, 
                       struct gvec *gvec,
                       size_t *iter);



#endif
