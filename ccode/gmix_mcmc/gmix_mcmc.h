/*
   styles
     1) coelliptical
       [row,col,e1,e2,T1,T2,....,p1,p2,....]

   Note, we use the coelliptical form when fitting exp/dev approximate
   models, but there is only one scale
       [row,col,e1,e2,T,p]

*/

#ifndef _GMIX_MCMC_HEADER_GUARS
#define _GMIX_MCMC_HEADER_GUARS

#include "mca.h"

#ifndef wlog
#define wlog(...) fprintf(stderr, __VA_ARGS__)
#endif

/*
   just do some error checking and call mca_make_guess

   check pars look like [row,col,T1,T2,T3,...,p1,p2,p3....]
   2*ngauss + 4
*/

struct mca_chain *gmix_mcmc_make_guess_coellip(double *centers, 
                                               double *widths,
                                               size_t npars, 
                                               size_t nwalkers);
struct gmix_mcmc {
    size_t ngauss;

    int cocenter;

};

#endif
