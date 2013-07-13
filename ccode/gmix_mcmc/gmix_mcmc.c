#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gmix.h"
#include "gmix_mcmc.h"


struct gmix_mcmc *gmix_mcmc_new(const struct gmix_mcmc_config *conf, long *flags)
{
    self=calloc(1, sizeof(struct gmix_mcmc));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct gmix_mcmc: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    // value type
    self->conf = (*conf);

}

struct mca_chain *gmix_mcmc_guess_simple(
        double row, double col,
        double T, double counts,
        size_t nwalkers)
{
    size_t npars=6;

    // note alloca, stack allocated
    double *centers=alloca(npars*sizeof(double));
    double *widths=alloca(npars*sizeof(double));

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



