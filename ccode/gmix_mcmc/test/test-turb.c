#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <float.h>

#include "../image.h"
#include "../gmix.h"
#include "../gmix_image.h"
#include "../gmix_mcmc.h"
#include "../admom.h"

#include "../gmix_sim1.h"

// these will hold pointers to data used in the fits
const struct image *_global_image_tmp;
struct gmix *_global_gmix_tmp;
double _global_ivar_tmp;

double lnprob(const double *pars, size_t npars, const void *void_data)
{
    double lnprob=0;
    long flags=0;
    double s2n_numer=0, s2n_denom=0;

    gmix_fill_coellip(_global_gmix_tmp, pars, npars, &flags);

    // flags are only set for conditions we want to
    // propagate back through the likelihood
    flags=gmix_image_loglike_ivar(_global_image_tmp,
                                  _global_gmix_tmp, 
                                  _global_ivar_tmp,
                                  &s2n_numer,
                                  &s2n_denom,
                                  &lnprob);


    return lnprob;
}
double *get_pars(double T, double counts,size_t *npars)
{
    size_t ngauss=2;
    (*npars)=2*ngauss+4;

    double *pars=calloc((*npars), sizeof(double));

    pars[0] = -1;
    pars[1] = -1;
    pars[2] = 0.0;
    pars[3] = 0.0;
    pars[4] = T*0.58;
    pars[5] = T*1.62;
    pars[6] = 0.60*counts;
    pars[7] = 0.40*counts;

    return pars;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr,"test-gauss s2n\n");
        exit(1);
    }
    double s2n=atof(argv[1]);

    char image_fname[] = "tmp/test-image.dat";
    char noisy_image_fname[] = "tmp/test-image-noisy.dat";
    char burn_fname[] = "tmp/chain-burnin.dat";
    char chain_fname[] = "tmp/chain.dat";
    //char fit_fname[] = "test-image-fit.dat";
    double counts=1;

    int nsub=16;
    size_t nwalkers=20;
    size_t burn_per_walker=200;
    size_t steps_per_walker=200;
    double a=2;

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);

    double *pars_true=NULL;
    double T=4;

    size_t npars=0;
    pars_true = get_pars(T,counts,&npars);
    size_t ngauss=(npars-4)/2;

    if (0 != system("mkdir -p tmp")) {
        fprintf(stderr,"could not make ./tmp");
        exit(1);
    }
    long flags=0;

    struct gmix *gmix_true=gmix_new_coellip(pars_true, npars, &flags);
    gmix_print(gmix_true,stderr);

    wlog("making turb sim\n");
    struct gmix_sim1 *sim=gmix_sim1_cocen_new(gmix_true,nsub);
    gmix_print(sim->gmix,stderr);

    wlog("storing image in '%s'\n", image_fname);
    image_write_file(sim->image, image_fname);

    gmix_sim1_add_noise(sim, s2n);

    double ivar = 1./(sim->skysig*sim->skysig);
    wlog("storing noisy image in '%s'\n", noisy_image_fname);
    image_write_file(sim->image, noisy_image_fname);

    // global variables hold pointers to data
    _global_image_tmp = (const struct image*) sim->image;
    _global_gmix_tmp = gmix_new(ngauss,&flags);
    _global_ivar_tmp = ivar;

    wlog("building guesses for %lu walkers\n", nwalkers);
    struct mca_chain *start_chain = gmix_mcmc_guess_turb_full(
            sim->gmix->data[0].row,
            sim->gmix->data[0].col,
            T,counts,nwalkers);

    wlog("creating burn-in chain for %lu steps per walker\n", burn_per_walker);
    struct mca_chain *burnin_chain=mca_chain_new(nwalkers, burn_per_walker, 
                                                 npars);
    wlog("creating chain for %lu steps per walker\n", steps_per_walker);
    struct mca_chain *chain=mca_chain_new(nwalkers, steps_per_walker, npars);


    wlog("    running burn-in\n");
    mca_run(burnin_chain, a, start_chain, &lnprob, NULL);

    wlog("    writing burn chain to %s\n", burn_fname);
    mca_chain_write_file(burnin_chain, burn_fname);

    wlog("    running chain\n");
    mca_run(chain, a, burnin_chain, &lnprob, NULL);

    wlog("    writing chain to %s\n", chain_fname);
    mca_chain_write_file(chain, chain_fname);

    wlog("brief stats\n");
    struct mca_stats *stats = mca_chain_stats(chain);
    mca_stats_write_brief(stats, stderr);

    gmix_true=gmix_free(gmix_true);
    _global_gmix_tmp=gmix_free(_global_gmix_tmp);
    sim=gmix_sim1_free(sim);
    start_chain=mca_chain_free(start_chain);
    burnin_chain=mca_chain_free(burnin_chain);
    chain=mca_chain_free(chain);
    stats=mca_stats_free(stats);

    return 0;
}
