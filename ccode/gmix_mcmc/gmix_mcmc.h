/*
   runn mcmc chains on images using gaussian mixtures
*/

#ifndef _GMIX_MCMC_HEADER_GUARD
#define _GMIX_MCMC_HEADER_GUARD

#include "gmix.h"
#include "gmix_mcmc_config.h"
#include "prob.h"
#include "mca.h"
#include "shear_prob.h"

// 2147483648
#define GMIX_MCMC_NOPOSITIVE 0x1
#define GMIX_MCMC_INIT 0x80000000

//#define GMIX_MCMC_MINPROB_USE 1.0e-8
#define GMIX_MCMC_MINPROB_USE 1.0e-16
#define GMIX_MCMC_MINFRAC_USE 0.80

#ifndef wlog
#define wlog(...) fprintf(stderr, __VA_ARGS__)
#endif

// the chains and stats structures
struct gmix_mcmc_chains {
    double mca_a; // ~2
    struct mca_chain *burnin_chain;
    struct mca_chain *chain;

    struct mca_stats *stats;
};

struct gmix_mcmc {
    // this is a value type
    struct gmix_mcmc_config conf;

    // contains references
    struct gmix_mcmc_chains chain_data;

    // e.g. PROB_NOSPLIT_ETA
    //enum prob_type prob_type;

    // probability calculator struct, e.g. prob_data_simple_gmix3_eta 
    // contains references
    // cast to (struct prob_data_base* ) to check the ->type field
    struct prob_data_base *prob; 

    // these can be set and reset
    const struct obs_list *obs_list;
    long use_obs_list;

    // for multi-band
    //const struct obs_list_list *obs_list_list;

    // only used when doing pqr method
    long nuse;
    double P;
    double Q[2];
    double R[2][2];

    long nuse_lensfit;
    double g[2];
    double gsens[2];

};

// you should caste prob_data_base to your actual type
//struct prob_data_base *prob_new(const struct gmix_mcmc_config *conf,
//                                long *flags);

// should we have one that copies an input config and one
// that loads a config file?
struct gmix_mcmc *gmix_mcmc_new(const struct gmix_mcmc_config *conf, long *flags);
struct gmix_mcmc *gmix_mcmc_free(struct gmix_mcmc *);
struct gmix_mcmc *gmix_mcmc_new_from_config(const char *name, long *flags);

void gmix_mcmc_set_obs_list(struct gmix_mcmc *self, const struct obs_list *obs_list);

long gmix_mcmc_calc_pqr(struct gmix_mcmc *self);
long gmix_mcmc_calc_lensfit(struct gmix_mcmc *self);
long gmix_mcmc_fill_prob1(struct gmix_mcmc *self,
                          struct shear_prob1 *shear_prob1);

void gmix_mcmc_run(struct gmix_mcmc *self,
                   struct mca_chain *guess);
void gmix_mcmc_run_draw_prior(struct gmix_mcmc *self);
struct mca_chain *gmix_mcmc_get_guess_prior(struct gmix_mcmc *self);

/*
void gmix_mcmc_run(struct gmix_mcmc *self,
                   double row, double col,
                   double T, double counts,
                   long *flags);
*/

/* older stuff */

/*
   just do some error checking and call mca_make_guess

   check pars look like [row,col,T1,T2,T3,...,p1,p2,p3....]
   2*ngauss + 4
*/

struct mca_chain *gmix_mcmc_make_guess_coellip(double *centers, 
                                               double *widths,
                                               size_t npars, 
                                               size_t nwalkers);

/* 
   make a turb psf guess with two fully free gaussians
*/

struct mca_chain *gmix_mcmc_guess_turb_full(
        double row, double col,
        double T, double counts,
        size_t nwalkers);


/* 

   this is a quick guess maker for models with a single scale and normalization
   The center guess for e1,e2 is 0,0

   The centers widths chosen may not always be what you want, check the code.
   widths on cen and ellip are 0.1. Width on T and p are 10 percent of the
   guess.

   center/pars are [row,col,e1,e2,T,counts]

*/


struct mca_chain *gmix_mcmc_guess_simple(
        double row, double col,
        double T, double counts,
        size_t nwalkers);


struct lognormal {
    double mean;
    double sigma;
    double logmean;
    double logivar;
};

void lognormal_set(struct lognormal *self, double mean, double sigma);
double lognormal_lnprob(struct lognormal *self, double x);


#endif
