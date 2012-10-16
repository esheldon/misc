#include <stdlib.h>
#include <stdio.h>
#include "gmix_mcmc.h"


struct mca_chain *gmix_mcmc_make_guess_coellip(double *centers, 
                                               double *widths,
                                               size_t npars, 
                                               size_t nwalkers)
{
    if ( ( (npars-4) % 2 ) != 0 ) {
        fprintf(stderr,"gmix_mcmc error: pars are wrong size for coelliptical\n");
        exit(EXIT_FAILURE);
    }
    return mca_make_guess(centers,
                          widths,
                          npars, 
                          nwalkers);
}

