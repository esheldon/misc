#ifndef _ADMOM_NOISE_HEADER_GUARD
#define _ADMOM_NOISE_HEADER_GUARD

/* 
  add gaussian noise to the image to get the requested S/N.  The S/N is defined
  as

     sum(weight*im)/sqrt(sum(weight)/skysig

  Returned are the skysig and the measured s/n which should be equivalent to
  the requested s/n to precision.
*/
void admom_add_noise(struct image *image, double s2n, const struct gauss *wt,
                     double *skysig, double *s2n_meas);
#endif
