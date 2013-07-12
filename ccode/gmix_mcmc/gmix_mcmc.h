/*
   styles
     1) coelliptical
       [row,col,e1,e2,T1,T2,....,p1,p2,....]

   Note, we use the coelliptical form when fitting exp/dev approximate
   models, but there is only one scale
       [row,col,e1,e2,T,p]

*/

#ifndef _GMIX_MCMC_HEADER_GUARD
#define _GMIX_MCMC_HEADER_GUARD

#include "gmix.h"
#include "mca.h"

#ifndef wlog
#define wlog(...) fprintf(stderr, __VA_ARGS__)
#endif


/* older stuff */

/*
   just do some error checking and call mca_make_guess

   check pars look like [row,col,T1,T2,T3,...,p1,p2,p3....]
   2*ngauss + 4
*/

struct mca_chain *gmix_mcmc_make_guess_coellip(double *centers, 
                                               double *widths,
                                               size_t npars, 
                                               size_t nwalkers);

/* 
   make a turb psf guess with two fully free gaussians
*/

struct mca_chain *gmix_mcmc_guess_turb_full(
        double row, double col,
        double T, double counts,
        size_t nwalkers);


/* 

   this is a quick guess maker for models with a single scale and normalization
   The center guess for e1,e2 is 0,0

   The centers widths chosen may not always be what you want, check the code.
   widths on cen and ellip are 0.1. Width on T and p are 10 percent of the
   guess.

   center/pars are [row,col,e1,e2,T,counts]

*/


struct mca_chain *gmix_mcmc_guess_simple(
        double row, double col,
        double T, double counts,
        size_t nwalkers);


struct lognormal {
    double mean;
    double sigma;
    double logmean;
    double logivar;
};

void lognormal_set(struct lognormal *self, double mean, double sigma);
double lognormal_lnprob(struct lognormal *self, double x);


#endif
