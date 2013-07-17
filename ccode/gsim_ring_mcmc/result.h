#ifndef _RESULT_HEADER_GUARD
#define _RESULT_HEADER_GUARD

#include "gmix_mcmc_config.h"
#include "gmix_mcmc.h"

// these are statistics derived from the mca chains and stats
// structures

struct result {
    // just copies of the parameters to simplify writing output
    double pars[GMIX_MCMC_MAXPARS];
    double cov[GMIX_MCMC_MAXPARS][GMIX_MCMC_MAXPARS];

    // other stats here....
    double P;
    double Q[2];
    double R[2][2];
};

//struct result *result_new(struct gmix_mcmc_chains *chain_data);
//struct result *result_free(struct result *self);

#endif
