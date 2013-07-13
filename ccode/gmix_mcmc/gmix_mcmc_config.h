#ifndef _GSIM_CONFIG_HEADER_GUARD
#define _GSIM_CONFIG_HEADER_GUARD

#include "gmix.h"
#include "prob.h"
#include "dist.h"

#define GMIX_MCMC_MAXPARS 6
#define GMIX_MCMC_MAXNAME 6

struct gmix_mcmc_config {
    long nwalkers;
    long burnin;
    long nstep;

    double mca_a;

    long psf_ngauss;
    long em_maxiter;
    double em_tol;

    // for now only fit a single model, but can expand this
    char fitmodel_name[GMIX_MCMC_MAXNAME];
    enum gmix_model fitmodel;
    long npars;

    // the type of probability calculation
    char prob_type_name[GMIX_MCMC_MAXNAME];
    enum prob_type prob_type;

    char shape_prior_name[GMIX_MCMC_MAXNAME];
    enum dist shape_prior;
    double shape_prior_pars[GMIX_MCMC_MAXPARS];

    char T_prior_name[GMIX_MCMC_MAXNAME];
    enum dist T_prior;
    double T_prior_pars[GMIX_MCMC_MAXPARS];

    char counts_prior_name[GMIX_MCMC_MAXNAME];
    enum dist counts_prior;
    double counts_prior_pars[GMIX_MCMC_MAXPARS];

    char cen_prior_name[GMIX_MCMC_MAXNAME];
    enum dist cen_prior;
    double cen_prior_pars[GMIX_MCMC_MAXPARS];
};

struct gmix_mcmc_config *gmix_mcmc_config_read(const char *name, enum cfg_status *status);
struct gmix_mcmc_config *gmix_mcmc_config_free(struct gmix_mcmc_config *self);
void gmix_mcmc_config_print(const struct gmix_mcmc_config *self, FILE *stream);

#endif
