#ifndef _GSIM_CONFIG_HEADER_GUARD
#define _GSIM_CONFIG_HEADER_GUARD

#include "gmix.h"
#include "prob.h"
#include "dist.h"

struct gmix_mcmc_config {
    long nwalkers;
    long burnin;
    long nstep;

    double mca_a;

    long psf_ngauss;
    long em_maxiter;
    double em_tol;

    // for now only fit a single model, but can expand this
    char fitmodel_name[20];
    enum gmix_model fitmodel;
    long npars;

    // the type of probability calculation
    char prob_type_name[20];
    enum prob_type prob_type;

    char shape_prior_name[20];
    enum dist shape_prior;

    char T_prior_name[20];
    enum dist T_prior;

    char counts_prior_name[20];
    enum dist counts_prior;
};

struct gmix_mcmc_config *gmix_mcmc_config_read(const char *name, enum cfg_status *status);
struct gmix_mcmc_config *gmix_mcmc_config_free(struct gmix_mcmc_config *self);
void gmix_mcmc_config_print(const struct gmix_mcmc_config *self, FILE *stream);

#endif
