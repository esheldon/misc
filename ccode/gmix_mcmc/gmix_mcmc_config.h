#ifndef _GSIM_CONFIG_HEADER_GUARD
#define _GSIM_CONFIG_HEADER_GUARD

#include "gmix.h"
#include "prob.h"
#include "dist.h"

struct gsim_mcmc_config {
    long nwalkers;
    long burnin;
    long nstep;

    double mca_a;

    long psf_ngauss;
    long em_maxiter;
    double em_tol;

    // for now only fit a single model, but can expand this
    enum gmix_model fitmodel;

    // the type of probability calculation
    enum prob_type prob_type;

    enum dist shape_prior;
    enum dist T_prior;
    enum dist counts_prior;
};

struct gsim_mcmc_config *gsim_mcmc_config_read(const char *name);
struct gsim_mcmc_config *gsim_mcmc_config_free(struct gsim_mcmc_config *self):

#endif
