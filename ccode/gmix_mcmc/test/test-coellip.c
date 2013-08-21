#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <float.h>

#include "../image.h"
#include "../gmix.h"
#include "../gmix_image.h"
#include "../gmix_mcmc.h"
#include "../randn.h"
#include "../admom.h"

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

    flags=gmix_image_loglike_ivar(_global_image_tmp,
                                  _global_gmix_tmp, 
                                  _global_ivar_tmp,
                                  &s2n_numer,
                                  &s2n_denom,
                                  &lnprob);


    return lnprob;
}
size_t get_pars_and_guess(double **pars_true, double **guess, double **widths)
{
    size_t ngauss=2;
    size_t npars=2*ngauss+4;

    (*pars_true)=calloc(npars, sizeof(double));
    (*guess)    =calloc(npars, sizeof(double));
    (*widths)   =calloc(npars, sizeof(double));

    (*pars_true)[0] = 20.;
    (*pars_true)[1] = 20.;
    (*pars_true)[2] = 0.2;
    (*pars_true)[3] = 0.1;
    (*pars_true)[4] = 16.;
    (*pars_true)[5] = 24.;
    (*pars_true)[6] = 0.6;
    (*pars_true)[7] = 0.4;

    (*widths)[0] = 1.0;
    (*widths)[1] = 1.0;
    (*widths)[2] = 0.1;
    (*widths)[3] = 0.1;
    (*widths)[4] = 0.1*(*pars_true)[4];
    (*widths)[5] = 0.1*(*pars_true)[5];
    (*widths)[6] = 0.1*(*pars_true)[6];
    (*widths)[7] = 0.1*(*pars_true)[7];

    (*guess)[0] = (*pars_true)[0] + (*widths)[0]*(randu()-0.5);
    (*guess)[1] = (*pars_true)[1] + (*widths)[1]*(randu()-0.5);
    (*guess)[2] = (*pars_true)[2] + (*widths)[2]*(randu()-0.5);
    (*guess)[3] = (*pars_true)[3] + (*widths)[3]*(randu()-0.5);
    (*guess)[4] = (*pars_true)[4] + (*widths)[4]*(randu()-0.5);
    (*guess)[5] = (*pars_true)[5] + (*widths)[5]*(randu()-0.5);
    (*guess)[6] = (*pars_true)[6] + (*widths)[6]*(randu()-0.5);
    (*guess)[7] = (*pars_true)[7] + (*widths)[7]*(randu()-0.5);

    return npars;
}

int main(int argc, char** argv)
{
    char image_fname[] = "tmp/test-image.dat";
    char noisy_image_fname[] = "tmp/test-image-noisy.dat";
    char burn_fname[] = "tmp/chain-burnin.dat";
    char chain_fname[] = "tmp/chain.dat";
    //char fit_fname[] = "test-image-fit.dat";
    size_t ngauss=2;
    size_t nrow=40, ncol=40;
    int nsub=1;
    size_t nwalkers=200;
    size_t burn_per_walker=200;
    size_t steps_per_walker=2000;
    double a=2;
    double skysig=0;
    double s2n=100;
    double s2n_meas=0;

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);

    double *pars_true=NULL, *guess=NULL, *widths=NULL;
    size_t npars = get_pars_and_guess(&pars_true, &guess, &widths);

    if (0 != system("mkdir -p tmp")) {
        fprintf(stderr,"could not make ./tmp");
        exit(1);
    }
    long flags=0;
    struct gmix *gmix_true=gmix_new_model(GMIX_COELLIP,pars_true, npars,&flags);


    // make the image and noisy image
    struct image* image = gmix_image_new(gmix_true, nrow, ncol, nsub);
    wlog("storing image in '%s'\n", image_fname);
    image_write_file(image, image_fname);

    struct image *noisy_im = image_new_copy(image);
    // need to fix this program
    admom_add_noise(noisy_im, s2n, &gmix_true->data[0], &skysig, &s2n_meas);
    double ivar = 1./(skysig*skysig);
    wlog("storing noisy image in '%s'\n", noisy_image_fname);
    image_write_file(noisy_im, noisy_image_fname);


    struct gmix *gmix_tmp=gmix_new(ngauss,&flags);

    // global variables hold pointers to data
    _global_image_tmp = (const struct image*) noisy_im;
    _global_gmix_tmp = gmix_tmp;
    _global_ivar_tmp = ivar;

    wlog("building guesses for %lu walkers\n", nwalkers);
    struct mca_chain *start_chain = gmix_mcmc_make_guess_coellip(guess, widths, npars, nwalkers);

    wlog("creating burn-in chain for %lu steps per walker\n", burn_per_walker);
    struct mca_chain *burnin_chain=mca_chain_new(nwalkers, burn_per_walker, npars);

    wlog("    running burn-in\n");
    mca_run(burnin_chain, a, start_chain, &lnprob, NULL);

    wlog("    writing burn chain to %s\n", burn_fname);
    mca_chain_write_file(burnin_chain, burn_fname);

    wlog("creating chain for %lu steps per walker\n", steps_per_walker);
    struct mca_chain *chain=mca_chain_new(nwalkers, steps_per_walker, npars);
    wlog("    running chain\n");
    mca_run(chain, a, burnin_chain, &lnprob, NULL);

    wlog("    writing chain to %s\n", chain_fname);
    mca_chain_write_file(chain, chain_fname);

    wlog("brief stats\n");
    struct mca_stats *stats = mca_chain_stats(chain);
    mca_stats_write_brief(stats, stderr);


    start_chain=mca_chain_free(start_chain);
    burnin_chain=mca_chain_free(burnin_chain);
    chain=mca_chain_free(chain);
    stats=mca_stats_free(stats);

    return 0;
}
