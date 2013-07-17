#include <stdio.h>
#include <stdlib.h>

#include "result.h"

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
