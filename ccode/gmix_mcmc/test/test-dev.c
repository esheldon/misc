#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <float.h>

#include "../image.h"
#include "../gmix.h"
#include "../gmix_image.h"
#include "../gmix_mcmc.h"
#include "../gmix_sim1.h"
#include "../mca.h"

struct fit_data {

    const struct image *image; // the input image
    double ivar;               // ivar for the image

    enum gmix_model par_type; // parameter array type

    struct gmix *obj;            // to be filled from pars array

    // optional
    const struct gmix *psf;      // pre-determined psf gmix
    struct gmix *conv;           // convolved gmix to be filled 
                                 // from psf and obj
};

struct fit_data 
*fit_data_new(const struct image *image,
              double ivar,
              enum gmix_model par_type,
              int ngauss, // ignored if par type is an approximate model
              const struct gmix *psf) // can be NULL
{
    long flags=0;
    struct fit_data *self=NULL;
    self=calloc(1,sizeof(struct fit_data));
    if (!self) {
        wlog("error: could not allocate struct "
             "fit_data.  %s: %d\n",__FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }

    self->image=image;
    self->ivar=ivar;
    self->par_type=par_type;
    self->psf=psf;

    if (par_type==GMIX_DEV) {
        self->obj=gmix_new(10,&flags);
    } else {
        self->obj=gmix_new(ngauss,&flags);
    }

    if (self->psf) {

        int ngauss_psf=self->psf->size;
        int ntot=self->obj->size*ngauss_psf;
        self->conv=gmix_new(ntot,&flags);

        if (!self->conv) {
            wlog("error: could not allocate %d gmix "
                 "%s: %d\n",ntot,__FILE__,__LINE__);
            exit(EXIT_FAILURE);
        }
    }
    return self;
}

struct fit_data 
*fit_data_free(struct fit_data *self)
{
    if (self) {
        self->obj=gmix_free(self->obj);
        self->conv=gmix_free(self->conv);
        free(self);
    }
    return NULL;
}


double lnprob(const double *pars, 
              size_t npars, 
              const void *void_data)
{
    double lnp=0;
    long flags=0;

    struct fit_data *data=(struct fit_data*) void_data;

    const struct gmix *gmix=NULL;

    if (data->par_type == GMIX_COELLIP) {
        gmix_fill_coellip(data->obj, pars, npars, &flags);
        if (flags != 0) {
            exit(EXIT_FAILURE);
        }
        gmix=data->obj;
    } else {
        if (data->par_type == GMIX_DEV) {
           gmix_fill_dev10(data->obj, pars, npars,&flags);
           if (flags != 0) {
                exit(EXIT_FAILURE);
           }
        } else if (data->par_type == GMIX_EXP) {
            gmix_fill_exp6(data->obj, pars, npars, &flags);
            if (flags != 0 ) {
                exit(EXIT_FAILURE);
            }
        } else {
            wlog("error: expected par_type of approx dev,exp or coellip "
                    "but got %d. %s: %d\n",
                    data->par_type,__FILE__,__LINE__);
            exit(EXIT_FAILURE);
        }
        gmix_convolve_fill(data->conv, data->obj, data->psf,&flags);
        if (flags !=0 ) {
            exit(EXIT_FAILURE);
        }
        gmix=data->conv;
    }
    // flags are only set for conditions we want to
    // propagate back through the likelihood
    double s2n_numer=0, s2n_denom=0;
    flags=gmix_image_loglike_ivar(data->image,
                                gmix,
                                data->ivar,
                                &s2n_numer,
                                &s2n_denom,
                                &lnp);

    return lnp;
}




// true parameters for a gapprox turb profile
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
// true parameters for a gapprox dev profile
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

    struct fit_data *psf_data = NULL;
    struct fit_data *obj_data = NULL;

    double Tpsf=4.;
    double T=Tpsf*2;

    char psf_image_fname[] = "tmp/test-psf-image.dat";
    char psf_noisy_image_fname[] = "tmp/test-psf-image-noisy.dat";
    char psf_burn_fname[] = "tmp/chain-burnin-psf.dat";
    char psf_chain_fname[] = "tmp/chain-psf.dat";


    char image_fname[] = "tmp/test-image.dat";
    char noisy_image_fname[] = "tmp/test-image-noisy.dat";
    char burn_fname[] = "tmp/chain-burnin.dat";
    char chain_fname[] = "tmp/chain.dat";
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

    long flags=0;
    struct gmix *gmix_true=gmix_new_model(GMIX_DEV,pars_true, npars_true,&flags);
    struct gmix *gmix_psf=gmix_new_model(GMIX_TURB,pars_psf_true, npars_psf_true,&flags);
    struct gmix *gmix_conv=gmix_convolve(gmix_true,gmix_psf,&flags);

    wlog("making sims\n");
    struct gmix_sim1 *psf_sim=gmix_sim1_cocen_new(gmix_psf,nsub);
    struct gmix_sim1 *obj_sim=gmix_sim1_cocen_new(gmix_conv,nsub);

    wlog("adding noise\n");
    gmix_sim1_add_noise(psf_sim, psf_s2n);
    gmix_sim1_add_noise(obj_sim, s2n);

    wlog("storing psf image in '%s'\n", psf_image_fname);
    image_write_file(psf_sim->image, psf_image_fname);
    wlog("storing noisy psf image in '%s'\n", psf_noisy_image_fname);
    image_write_file(psf_sim->image, psf_noisy_image_fname);
    wlog("storing image in '%s'\n", image_fname);
    image_write_file(obj_sim->image, image_fname);
    wlog("storing noisy image in '%s'\n", noisy_image_fname);
    image_write_file(obj_sim->image, noisy_image_fname);


    //
    // process psf
    //

    int ngauss_psf=2;
    double psf_ivar=1./(psf_sim->skysig*psf_sim->skysig);
    psf_data=fit_data_new(psf_sim->image,
                          psf_ivar,
                          GMIX_COELLIP,
                          ngauss_psf,
                          NULL);

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



    wlog("    running psf burn-in\n");
    mca_run(psf_burnin_chain, a, psf_start_chain, &lnprob, psf_data);
    wlog("    running psf chain\n");
    mca_run(psf_chain, a, psf_burnin_chain, &lnprob, psf_data);

    wlog("psf brief stats\n");
    struct mca_stats *psf_stats = mca_chain_stats(psf_chain);
    mca_stats_write_brief(psf_stats, stderr);


    //
    // process convolved image
    //

    int ngauss_obj=3;

    double ivar=1./(obj_sim->skysig*obj_sim->skysig);
    obj_data=fit_data_new(obj_sim->image,
                          ivar,
                          GMIX_DEV,
                          ngauss_obj,
                          psf_data->obj);


    wlog("building dev guesses for %lu walkers\n", nwalkers);
    struct mca_chain *start_chain = gmix_mcmc_guess_simple(
            obj_sim->gmix->data[0].row,
            obj_sim->gmix->data[0].col,
            T,counts,nwalkers);

    wlog("creating burn-in chain for %lu steps per walker\n", 
         burn_per_walker);
    size_t npars_fit=MCA_CHAIN_NPARS(start_chain);
    struct mca_chain *burnin_chain=mca_chain_new(
            nwalkers, burn_per_walker, npars_fit);
    wlog("creating chain for %lu steps per walker\n", steps_per_walker);
    struct mca_chain *chain=
        mca_chain_new(nwalkers, steps_per_walker, npars_fit);



    wlog("    running burn-in\n");
    mca_run(burnin_chain, a, start_chain, &lnprob, obj_data);
    wlog("    running chain\n");
    mca_run(chain, a, burnin_chain, &lnprob, obj_data);

    wlog("brief stats\n");
    struct mca_stats *stats = mca_chain_stats(chain);
    mca_stats_write_brief(stats, stderr);




    double s2n_meas=gmix_image_s2n_ivar(obj_sim->image,
                                        obj_data->conv,
                                        ivar,
                                        &flags);
    wlog("s2n measured: %.2f\n", s2n_meas);

    wlog("    writing psf burn chain to %s\n", psf_burn_fname);
    mca_chain_write_file(psf_burnin_chain, psf_burn_fname);
    wlog("    writing psf chain to %s\n", psf_chain_fname);
    mca_chain_write_file(psf_chain, psf_chain_fname);

    wlog("    writing burn chain to %s\n", burn_fname);
    mca_chain_write_file(burnin_chain, burn_fname);
    wlog("    writing chain to %s\n", chain_fname);
    mca_chain_write_file(chain, chain_fname);

    psf_data=fit_data_free(psf_data);
    obj_data=fit_data_free(obj_data);

    return 0;
}
