#ifndef _RESULT_HEADER_GUARD
#define _RESULT_HEADER_GUARD

#include "mca.h"

struct result {
    double mca_a; // ~2
    struct mca_chain *burnin_chain;
    struct mca_chain *chain;

    struct mca_stats *stats;

    // other stats here....
    double P;
    double Q[2];
    double R[2][2];
};

struct result *result_new(long nwalkers, long burnin, long nstep, long npars, double a);
struct result *result_free(struct result *self);

#endif
