#include <stdio.h>
#include <stdlib.h>

#include "result.h"

void result_calc(struct result *self, const struct gmix_mcmc_chains *chain_data)
{
    self->npars = MCA_CHAIN_NPARS(chain_data->chain);
    memcpy(self->pars, chain_data->stats.mean, chain_data->stats.npars);
    memcpy(self->cov, chain_data->stats.cov, chain_data->stats.npars*chain_data->stats.npars);

}

void result_print(struct resut *self, FILE* stream)
{

}
/*
struct result *result_new(long nwalkers, long burnin, long nstep, long npars, double mca_a)
{
    struct result *self=calloc(1,sizeof(struct result));
    if (!self) {
        fprintf(stderr, "Could not alloc struct result: %s: %d",
                __FILE__,__LINE__);
        exit(1);
    }

    self->burnin_chain  = mca_chain_new(nwalkers, burnin, npars);
    self->chain         = mca_chain_new(nwalkers, nstep, npars);
    self->stats         = mca_stats_new(npars);
    self->mca_a         = mca_a;

    return self;
}

struct result *result_free(struct result *self)
{
    if (self) {
        self->burnin_chain = mca_chain_free(self->burnin_chain);
        self->chain        = mca_chain_free(self->chain);
        self->stats        = mca_stats_free(self->stats);

        free(self);
        self=NULL;
    }
    return self;
}
*/
