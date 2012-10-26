#include "image.h"
#include "image_rand.h"
#include "gmix.h"
#include "gmix_image_rand.h"

int gmix_image_add_randn(struct image *image, 
                         double s2n,
                         const struct gmix *gmix,
                         double *skysig) 
{
    int flags=0;

    if (!gmix_verify(gmix)) {
        flags |= GMIX_IMAGE_NEGATIVE_DET;
        goto _gmix_image_add_noise_bail;
    }

    double tskysig=1;
    double s2n_first_pass=gmix_image_s2n(image,tskysig,gmix,&flags);
    if (flags!=0) {
        goto _gmix_image_add_noise_bail;
    }
    (*skysig) = s2n_first_pass/s2n * tskysig;

    image_add_randn(image, (*skysig));

_gmix_image_add_noise_bail:
    return flags;
}


