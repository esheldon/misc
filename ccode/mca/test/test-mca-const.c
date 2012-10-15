/*
   test fitting a constant
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "../mca.h"

struct mydata {
    size_t ndata;
    const double *data;
    double ivar; // same error for each
};

double lnprob(double *pars, size_t npars, void *userdata)
{
    double chi2=0, diff=0;
    const struct mydata *mydata = userdata;

    for (size_t i=0; i<mydata->ndata; i++) {
        diff = mydata->data[i]-pars[0];
        chi2 += diff*diff*mydata->ivar;
    }

    double lnprob = -0.5*chi2;

    return lnprob;
}

int main(int argc, char **argv)
{
    double a=2;
    time_t tm;

    size_t ndata=100;
    size_t npars=1;
    size_t nwalkers=10;
    size_t burn_per_walker=400;
    size_t steps_per_walker=500;

    double truepars[1] = {1};
    double guess[1] = {0};
    double ballsize[1] = {0};
    double fracerr=0.1;

    double err=fracerr*truepars[0];

    fprintf(stderr,"nwalkers:  %lu\n", nwalkers);
    fprintf(stderr,"burn per:  %lu\n", burn_per_walker);
    fprintf(stderr,"steps per: %lu\n", steps_per_walker);
    fprintf(stderr,"npars:     %lu\n", npars);
    fprintf(stderr,"truth:     %.16g\n", truepars[0]);
    fprintf(stderr,"npoints:   %lu\n", ndata);
    fprintf(stderr,"err per:   %.16g\n", err);
    fprintf(stderr,"expect err on mean: %.16g\n", err/sqrt(ndata));


    // set up the random number generator
    (void) time(&tm);
    srand48((long) tm);

    // set up the data
    double *data = malloc(ndata*sizeof(double));
    for (size_t i=0; i<ndata; i++) {
        data[i] = truepars[0] + err*mca_randn();
    }


    guess[0] = truepars[0] + err*mca_randn();
    ballsize[0] = 1.0;

    struct mca_chain *guesses=mca_make_guess(guess, ballsize, npars, nwalkers);

    struct mydata mydata;
    mydata.ndata = ndata;
    mydata.data = (const double*) data;
    mydata.ivar = 1/(err*err);

    struct mca_chain *burn_chain=mca_chain_new(nwalkers,burn_per_walker,npars);
    mca_run(a, guesses, burn_chain, &lnprob, (void*) &mydata);
    //mca_chain_print(burn_chain, stderr);

    struct mca_chain *chain=mca_chain_new(nwalkers,steps_per_walker,npars);
    mca_run(a, burn_chain, chain, &lnprob, (void*) &mydata);

    struct mca_stats *stats=mca_chain_stats(chain);
    fprintf(stderr,"\nStats:\n");
    mca_stats_print(stats,stderr);
    fprintf(stderr,"\nStats full:\n");
    mca_stats_print_full(stats,stderr);

    mca_chain_print(chain, stdout);

    guesses=mca_chain_del(guesses);
    stats=mca_stats_del(stats);
    free(data);
}

