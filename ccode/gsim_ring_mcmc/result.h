#ifndef _RESULT_HEADER_GUARD
#define _RESULT_HEADER_GUARD

#include "gmix_mcmc_config.h"
#include "gmix_mcmc.h"

// these are statistics derived from the mca chains and stats
// structures

struct result {
    double P;
    double Q[2];
    double R[2][2];
};

//struct result *result_new(struct gmix_mcmc_chains *chain_data);
//struct result *result_free(struct result *self);

void result_calc(struct result *self, const struct gmix_mcmc_chains *chain_data);
void result_print(const struct result *self, FILE* stream);

#endif
