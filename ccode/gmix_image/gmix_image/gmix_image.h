#ifndef _GMIX_IMAGE_HEADER_GUARD
#define _GMIX_IMAGE_HEADER_GUARD

#include "image.h"
#include "gvec.h"

#define GMIX_ERROR_NEGATIVE_DET 0x1
#define GMIX_ERROR_MAXIT 0x2

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

#endif
