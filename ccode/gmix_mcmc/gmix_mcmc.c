#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "prob.h"
#include "gmix.h"
#include "gmix_mcmc_config.h"
#include "gmix_mcmc.h"

// we can generalize these later
// you should caste prob_data_base to your actual type
static struct prob_data_base *prob_new_generic(const struct gmix_mcmc_config *conf, long *flags)
{

    struct prob_data_simple_gmix3_eta *prob=NULL;

    struct dist_gauss cen_prior={0};

    struct dist_gmix3_eta shape_prior={0};

    struct dist_lognorm T_prior={0};
    struct dist_lognorm counts_prior={0};

    fprintf(stderr,"loading prob gmix3_eta\n");

    if (conf->cen_prior_npars != 2
            || conf->T_prior_npars != 2
            || conf->counts_prior_npars != 2
            || conf->shape_prior_npars != 6) {

        fprintf(stderr,"wrong npars: %s: %d\n",
                __FILE__,__LINE__);
        *flags |= DIST_WRONG_NPARS;
        return NULL;
    }
    dist_gauss_fill(&cen_prior,
                    conf->cen_prior_pars[0],
                    conf->cen_prior_pars[1]);
    dist_lognorm_fill(&T_prior, 
                      conf->T_prior_pars[0],
                      conf->T_prior_pars[1]);
    dist_lognorm_fill(&counts_prior,
                      conf->counts_prior_pars[0],
                      conf->counts_prior_pars[1]);
    dist_gmix3_eta_fill(&shape_prior,
                        conf->shape_prior_pars[0],  // sigma1
                        conf->shape_prior_pars[1],  // sigma2
                        conf->shape_prior_pars[2],  // sigma3
                        conf->shape_prior_pars[3],  // p1
                        conf->shape_prior_pars[4],  // p2
                        conf->shape_prior_pars[5]); // p3


    // priors get copied
    prob=prob_data_simple_gmix3_eta_new(conf->fitmodel,
                                        conf->psf_ngauss,

                                        &cen_prior,
                                        &cen_prior,

                                        &shape_prior,

                                        &T_prior,
                                        &counts_prior,
                                        flags);

    return (struct prob_data_base *) prob;
}

// can generalize this
static struct prob_data_base *prob_free_generic(struct prob_data_base *prob)
{
    prob_data_simple_gmix3_eta_free( (struct prob_data_simple_gmix3_eta *) prob);
    return NULL;
}


static void create_chain_data(struct gmix_mcmc *self)
{

    self->chain_data.mca_a = self->conf.mca_a;
    self->chain_data.burnin_chain = mca_chain_new(self->conf.nwalkers,
                                                  self->conf.burnin,
                                                  self->conf.npars);
    self->chain_data.chain = mca_chain_new(self->conf.nwalkers,
                                           self->conf.nstep,
                                           self->conf.npars);
    self->chain_data.stats = mca_stats_new(self->conf.npars);

}
static void free_chain_data(struct gmix_mcmc *self)
{
    self->chain_data.burnin_chain = mca_chain_free(self->chain_data.burnin_chain);
    self->chain_data.chain = mca_chain_free(self->chain_data.chain);
    self->chain_data.stats = mca_stats_free(self->chain_data.stats);
}


struct gmix_mcmc *gmix_mcmc_new(const struct gmix_mcmc_config *conf, long *flags)
{
    struct gmix_mcmc *self=calloc(1, sizeof(struct gmix_mcmc));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct gmix_mcmc: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    // value type
    self->conf = (*conf);

    // cast to (prob_data_base *) to check the type
    // use PROB_GET_TYPE macro
    self->prob = prob_new_generic(conf, flags);
    if (*flags != 0) {
        goto _gmix_mcmc_new_bail;
    }

    create_chain_data(self);

_gmix_mcmc_new_bail:
    if (*flags != 0)  {
       self=gmix_mcmc_free(self);
    }

    return self;
}
struct gmix_mcmc *gmix_mcmc_new_from_config(const char *name, long *flags)
{

    struct gmix_mcmc *self=NULL;
    struct gmix_mcmc_config conf={0};

    *flags=gmix_mcmc_config_load(&conf, name);
    if (*flags != 0) {
        goto _gmix_mcmc_new_from_config_bail;
    }

    self = gmix_mcmc_new(&conf, flags);

_gmix_mcmc_new_from_config_bail:
    if (*flags != 0) {
        self=gmix_mcmc_free(self);
    }

    return self;
}
struct gmix_mcmc *gmix_mcmc_free(struct gmix_mcmc *self)
{
    if (self) {
        free_chain_data(self);
        self->prob=prob_free_generic((struct prob_data_base *) self->prob);

        free(self);
        self=NULL;
    }
    return self;
}

void gmix_mcmc_set_obs_list(struct gmix_mcmc *self, const struct obs_list *obs_list)
{
    self->obs_list=obs_list;
}


// can generalize this by adding a callback to the prob struct
// and allowing different guess sizes/values for different model types

// note flags get "lost" here, you need good error messages
static double get_lnprob(const double *pars, size_t npars, const void *data)
{
    double lnprob=0, s2n_numer=0, s2n_denom=0;
    long flags=0;

    struct gmix_mcmc *self=(struct gmix_mcmc *)data;

    prob_simple_gmix3_eta_calc((struct prob_data_simple_gmix3_eta *)self->prob,
                                self->obs_list,
                                pars, npars,
                                &s2n_numer, &s2n_denom,
                                &lnprob, &flags);

    return lnprob;
}

// we will also need one for multi-band processing
void gmix_mcmc_run(struct gmix_mcmc *self,
                   double row, double col,
                   double T, double counts,
                   long *flags)
{
    if (!self->obs_list) {
        fprintf(stderr,"gmix_mcmc->obs_list is not set!: %s: %d\n",
                __FILE__,__LINE__);
        *flags |= GMIX_MCMC_INIT;
        return;
    }

    // need to generalize this
    long nwalkers=MCA_CHAIN_NWALKERS(self->chain_data.chain);
    struct mca_chain *guess=gmix_mcmc_guess_simple(row, col,
                                                   T, counts,
                                                   nwalkers);

    mca_run(self->chain_data.burnin_chain,
            self->chain_data.mca_a,
            guess,
            &get_lnprob,
            self);

    mca_run(self->chain_data.chain,
            self->chain_data.mca_a,
            self->chain_data.burnin_chain,
            &get_lnprob,
            self);
    guess=mca_chain_free(guess);
}


struct mca_chain *gmix_mcmc_guess_simple(
        double row, double col,
        double T, double counts,
        size_t nwalkers)
{
    size_t npars=6;
    double centers[6], widths[6];

    centers[0]=row;
    centers[1]=col;
    centers[2]=0.;
    centers[3]=0.;
    centers[4]=T;
    centers[5]=counts;

    widths[0] = 0.1;
    widths[1] = 0.1;
    widths[2] = 0.05;
    widths[3] = 0.05;
    widths[4] = 0.1*centers[4];
    widths[5] = 0.1*centers[5];

    return mca_make_guess(centers,
                          widths,
                          npars, 
                          nwalkers);
}





struct mca_chain *gmix_mcmc_guess_turb_full(
        double row, double col,
        double T, double counts,
        size_t nwalkers)
{
    size_t ngauss=2;
    size_t npars=2*ngauss+4;

    // note alloca, stack allocated
    double *centers=alloca(npars*sizeof(double));
    double *widths=alloca(npars*sizeof(double));

    centers[0]=row;
    centers[1]=col;
    centers[2]=0.;
    centers[3]=0.;
    centers[4]=T*0.58;
    centers[5]=T*1.62;
    centers[6]=counts*0.60;
    centers[7]=counts*0.40;

    widths[0] = 0.1;
    widths[1] = 0.1;
    widths[2] = 0.05;
    widths[3] = 0.05;
    widths[4] = 0.1*centers[4];
    widths[5] = 0.1*centers[5];
    widths[6] = 0.1*centers[6];
    widths[7] = 0.1*centers[7];

    return mca_make_guess(centers,
                          widths,
                          npars, 
                          nwalkers);
}



// this is the more generic one
struct mca_chain *gmix_mcmc_make_guess_coellip(double *centers, 
                                               double *widths,
                                               size_t npars, 
                                               size_t nwalkers)
{
    if ( ( (npars-4) % 2 ) != 0 ) {
        fprintf(stderr,
                "gmix_mcmc error: pars are wrong size for coelliptical\n");
        exit(EXIT_FAILURE);
    }
    return mca_make_guess(centers,
                          widths,
                          npars, 
                          nwalkers);
}


void lognormal_set(struct lognormal *self, double mean, double sigma)
{
    self->mean=mean;
    self->sigma=sigma;

    double rat=(sigma*sigma)/(mean*mean);
    double logvar = log(1 + rat);

    self->logmean = log(mean) - 0.5*logvar;
    self->logivar = 1./logvar;
}

double lognormal_lnprob(
        struct lognormal *self,
        double x)
{
    double logx = log(x);
    double ldiff = logx-self->logmean;
    double chi2 = -0.5*self->logivar*ldiff*ldiff;

    return chi2 - logx;
}
double lognormal_prob(
        struct lognormal *self,
        double x)
{
    return exp(lognormal_lnprob(self,x));
}



