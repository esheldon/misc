#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gmix.h"
#include "gmix_mcmc.h"

struct mca_chain *gmix_mcmc_guess_simple(double row, double col,
                                         double T, double p,
                                         size_t nwalkers)
{
    int npars=6;
    // note alloca, stack allocated
    double *centers=alloca(npars*sizeof(double));
    double *widths=alloca(npars*sizeof(double));

    centers[0]=row;
    centers[1]=col;
    centers[2]=0.;
    centers[3]=0.;
    centers[4]=T;
    centers[5]=p;

    widths[0] = 0.1;
    widths[1] = 0.1;
    widths[2] = 0.05;
    widths[3] = 0.05;
    widths[4] = 0.1*T;
    widths[5] = 0.1*p;

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

