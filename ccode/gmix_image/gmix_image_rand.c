#include "image.h"
#include "image_rand.h"
#include "gmix.h"
#include "gmix_image.h"
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
    double tivar=1./(tskysig*tskysig);
    double s2n_numer=0, s2n_denom=0, loglike=0;

    flags=gmix_image_loglike_ivar(image, 
                                  gmix, 
                                  tivar,
                                  &s2n_numer,
                                  &s2n_denom,
                                  &loglike);

    if (flags!=0) {
        goto _gmix_image_add_noise_bail;
    }

    double s2n_first_pass = s2n_numer/sqrt(s2n_denom);

    (*skysig) = s2n_first_pass/s2n * tskysig;

    image_add_randn(image, (*skysig));

#if 0
    tivar = 1/( (*skysig)*(*skysig) );
    flags=gmix_image_loglike_ivar(image, 
                                  gmix, 
                                  tivar,
                                  &s2n_numer,
                                  &s2n_denom,
                                  &loglike);

    double s2n_second_pass = s2n_numer/sqrt(s2n_denom);
    fprintf(stderr,"s2n check: %g\n", s2n_second_pass);
#endif

_gmix_image_add_noise_bail:
    return flags;
}


