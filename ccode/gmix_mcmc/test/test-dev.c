#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <float.h>

#include "../image.h"
#include "../gmix.h"
#include "../gmix_image.h"
#include "../gmix_mcmc.h"
#include "../gmix_sim.h"
#include "../mca.h"

struct psf_data {
    const struct image *image;
    double ivar;
    struct gmix *psf;
};
struct obj_data {
    const struct image *image;
    double ivar;
    const struct gmix *psf;
    struct gmix *obj;
};


double lnprob_psf(const double *pars, 
                  size_t npars, 
                  const void *void_data)
{
    double lnprob=0;
    int flags=0;

    struct psf_data *data=(struct psf_data*) void_data;
    if (!gmix_fill_coellip(data->psf, pars, npars) ) {
        exit(EXIT_FAILURE);
    }
    lnprob=gmix_image_loglike(data->image,
                              data->psf,
                              data->ivar,
                              &flags);

    return lnprob;
}

double lnprob_dev(const double *pars, 
                  size_t npars, 
                  const void *void_data)
{
    double lnprob=0;
    int flags=0;

    struct obj_data *data=(struct obj_data*) void_data;
    if (!gmix_fill_dev(data->obj, pars, npars) ) {
        exit(EXIT_FAILURE);
    }

    struct gmix *conv=gmix_convolve(data->obj, data->psf);

    // flags are only set for conditions we want to
    // propagate back through the likelihood
    lnprob=gmix_image_loglike(data->image,
                              conv,
                              data->ivar,
                              &flags);

    conv=gmix_free(conv);
    return lnprob;
}

// this is for gapprox turb profile
size_t get_psf_pars(double T, double **pars)
{
    size_t npars=6;
    (*pars)=calloc(npars, sizeof(double));

    (*pars)[0] = -1;
    (*pars)[1] = -1;
    (*pars)[2] = 0.0;
    (*pars)[3] = 0.0;
    (*pars)[4] = T;
    (*pars)[5] = 1.;

    return npars;
}
size_t get_pars(double T, double **pars)
{
    size_t npars=6;

    (*pars)=calloc(npars, sizeof(double));
    (*pars)[0] = -1;
    (*pars)[1] = -1;
    (*pars)[2] = 0.2;
    (*pars)[3] = 0.1;
    (*pars)[4] = T;
    (*pars)[5] = 1.;

    return npars;
}


int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr,"test-gauss psf_s2n s2n\n");
        exit(1);
    }
    double psf_s2n=atof(argv[1]);
    double s2n=atof(argv[2]);

    struct psf_data psf_data = {0};
    struct obj_data obj_data = {0};

    double Tpsf=4.;
    double T=Tpsf*2;

    char psf_image_fname[] = "tmp/test-psf-image.dat";
    char psf_noisy_image_fname[] = "tmp/test-psf-image-noisy.dat";
    char psf_burn_fname[] = "tmp/chain-burnin-psf.dat";
    char psf_chain_fname[] = "tmp/chain-psf.dat";


    /*
    char image_fname[] = "tmp/test-image.dat";
    char noisy_image_fname[] = "tmp/test-image-noisy.dat";
    char burn_fname[] = "tmp/chain-burnin.dat";
    char chain_fname[] = "tmp/chain.dat";
    */
    //char fit_fname[] = "test-image-fit.dat";

    //size_t nrow=25, ncol=25;
    int nsub=16;
    size_t nwalkers=20;
    size_t burn_per_walker=200;
    size_t steps_per_walker=200;
    double a=2;

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);


    double *pars_true=NULL, *pars_psf_true=NULL;

    size_t npars_psf_true=get_psf_pars(Tpsf, &pars_psf_true);
    size_t npars_true=get_pars(T, &pars_true);
    double psf_counts=pars_psf_true[5];
    double counts=pars_true[5];

    if (0 != system("mkdir -p tmp")) {
        fprintf(stderr,"could not make ./tmp");
        exit(1);
    }

    struct gmix *gmix_true=gmix_make_dev(pars_true, npars_true);
    struct gmix *gmix_psf=gmix_make_turb(pars_psf_true, npars_psf_true);
    struct gmix *gmix_conv=gmix_convolve(gmix_true,gmix_psf);

    wlog("making psf sim\n");
    struct gmix_sim *psf_sim=gmix_sim_cocen_new(gmix_psf,nsub);
    wlog("making dev convolved sim\n");
    struct gmix_sim *obj_sim=gmix_sim_cocen_new(gmix_conv,nsub);

    // make the image and noisy image
    wlog("storing image in '%s'\n", psf_image_fname);
    image_write_file(obj_sim->image, psf_image_fname);

    gmix_sim_add_noise(psf_sim, psf_s2n);
    gmix_sim_add_noise(obj_sim, s2n);

    wlog("storing noisy image in '%s'\n", psf_noisy_image_fname);
    image_write_file(obj_sim->image, psf_noisy_image_fname);


    wlog("building turb guesses for %lu walkers\n", nwalkers);
    struct mca_chain *psf_start_chain = gmix_mcmc_guess_turb_full(
            psf_sim->gmix->data[0].row,
            psf_sim->gmix->data[0].col,
            Tpsf,psf_counts,nwalkers);

    wlog("creating burn-in chain for %lu steps per walker\n", 
         burn_per_walker);
    size_t npars_psf_fit=2*2+4;
    struct mca_chain *psf_burnin_chain=mca_chain_new(
            nwalkers, burn_per_walker, npars_psf_fit);
    wlog("creating chain for %lu steps per walker\n", steps_per_walker);
    struct mca_chain *psf_chain=
        mca_chain_new(nwalkers, steps_per_walker, npars_psf_fit);


    psf_data.image = (const struct image*) psf_sim->image;
    psf_data.psf=gmix_new(2);
    psf_data.ivar=1./(psf_sim->skysig*psf_sim->skysig);

    wlog("    running psf burn-in\n");
    mca_run(psf_burnin_chain, a, psf_start_chain, &lnprob_psf, &psf_data);
    wlog("    running psf chain\n");
    mca_run(psf_chain, a, psf_burnin_chain, &lnprob_psf, &psf_data);

    wlog("psf brief stats\n");
    struct mca_stats *psf_stats = mca_chain_stats(psf_chain);
    mca_stats_write_brief(psf_stats, stderr);



    obj_data.image = (const struct image*) obj_sim->image;
    obj_data.psf=(const struct gmix *) psf_data.psf;
    obj_data.obj=gmix_new(3);
    obj_data.ivar=1./(obj_sim->skysig*obj_sim->skysig);




    wlog("building dev guesses for %lu walkers\n", nwalkers);
    struct mca_chain *start_chain = gmix_mcmc_guess_gapprox(
            obj_sim->gmix->data[0].row,
            obj_sim->gmix->data[0].col,
            T,counts,nwalkers);








    wlog("    writing psf burn chain to %s\n", psf_burn_fname);
    mca_chain_write_file(psf_burnin_chain, psf_burn_fname);
    wlog("    writing psf chain to %s\n", psf_chain_fname);
    mca_chain_write_file(psf_chain, psf_chain_fname);

    return 0;
}
