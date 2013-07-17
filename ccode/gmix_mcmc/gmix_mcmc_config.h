#ifndef _GMIX_MCMC_CONFIG_HEADER_GUARD
#define _GMIX_MCMC_CONFIG_HEADER_GUARD

#include "gmix.h"
#include "prob.h"
#include "dist.h"

#define GMIX_MCMC_MAXPARS 6
#define GMIX_MCMC_MAXNAME 20

// this is a value type
struct gmix_mcmc_config {
    // MCA for the object measurement
    long nwalkers;
    long burnin;
    long nstep;

    double mca_a;

    // we use EM for the PSF fitting
    long psf_ngauss;
    long em_maxiter;
    double em_tol;

    // for now only fit a single model, but can expand this to an array
    char fitmodel_name[GMIX_MCMC_MAXNAME];
    enum gmix_model fitmodel;
    long nmodel;
    long npars;

    // the type of probability calculation
    char prob_type_name[GMIX_MCMC_MAXNAME];
    enum prob_type prob_type;

    char shape_prior_name[GMIX_MCMC_MAXNAME];
    enum dist shape_prior;
    double shape_prior_pars[GMIX_MCMC_MAXPARS];
    size_t shape_prior_npars;

    char T_prior_name[GMIX_MCMC_MAXNAME];
    enum dist T_prior;
    double T_prior_pars[GMIX_MCMC_MAXPARS];
    size_t T_prior_npars;

    char counts_prior_name[GMIX_MCMC_MAXNAME];
    enum dist counts_prior;
    double counts_prior_pars[GMIX_MCMC_MAXPARS];
    size_t counts_prior_npars;

    char cen_prior_name[GMIX_MCMC_MAXNAME];
    enum dist cen_prior;
    double cen_prior_pars[GMIX_MCMC_MAXPARS];
    size_t cen_prior_npars;
};

long gmix_mcmc_config_load(struct gmix_mcmc_config *self, const char *name);
void gmix_mcmc_config_print(const struct gmix_mcmc_config *self, FILE *stream);

#endif
