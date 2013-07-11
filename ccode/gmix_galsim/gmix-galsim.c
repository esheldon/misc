/*

   This is a quick app to run on some high s/n galsim 
   images.

   Only coellip for now.

*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "admom.h"
#include "image.h"
#include "image_rand.h"
#include "gmix.h"
#include "gmix_image.h"
#include "gmix_image_fits.h"
#include "gmix_mcmc.h"
#include "mca.h"
#include "shape.h"

#include "config.h"

#include "fitsio.h"

struct config {
    double a;

    char *obj_model;
    enum gmix_model obj_type;
    char *psf_model;
    enum gmix_model psf_type;

    int ngauss;
    int ngauss_psf;

    int nwalkers_psf;
    int burn_per_walker_psf;
    int steps_per_walker_psf;

    int nwalkers;
    int burn_per_walker;
    int steps_per_walker;

    double skysig;
    double skysig_psf;
};

struct object {
    // location on object grid
    size_t orow;
    size_t ocol;

    // guess at center, just middle of region for each object
    double rowguess;
    double colguess;

    // boundary in main image
    struct image_mask mask;
};

void object_write(struct object *self, FILE *fptr)
{
    fprintf(fptr,"%lu %lu %lf %lf\n",
            self->orow,self->ocol,self->rowguess,self->colguess);
    image_mask_print(&self->mask, fptr);
}


struct fitter {

    struct image *image; // data not owned, only change the mask!
    double ivar;         // ivar for the image

    double a;

    enum gmix_model par_type;

    // we can keep these around
    struct mca_chain *burnin_chain;
    struct mca_chain *chain;

    // replace each time?
    struct mca_stats *stats;

    struct gmix *obj;            // to be filled from pars array

    // optional
    const struct gmix *psf;      // pre-determined psf gmix
    struct gmix *conv;           // convolved gmix to be filled 
                                 // from psf and obj

};


struct fitter_ce {

    struct image *image; // data not owned, only change the mask!
    double ivar;         // ivar for the image

    double a;

    // we can keep these around
    struct mca_chain *burnin_chain;
    struct mca_chain *chain;

    // replace each time?
    struct mca_stats *stats;

    struct gmix *obj;            // to be filled from pars array

    // optional
    const struct gmix *psf;      // pre-determined psf gmix
    struct gmix *conv;           // convolved gmix to be filled 
                                 // from psf and obj

};

struct fitters_ce {
    struct fitter_ce *psf_fitter;
    struct fitter_ce *fitter;
};

struct fitter_ce *fitter_ce_new(
        struct image *image,
        double ivar,
        double a,
        int nwalkers, 
        int burn_per_walker, 
        int steps_per_walker,
        int ngauss,
        const struct gmix *psf)
{
    struct fitter_ce *self=calloc(1,sizeof(struct fitter_ce));
    if (!self) {
        fprintf(stderr,"could not allocate struct fitter_ce\n");
        exit(EXIT_FAILURE);
    }

    self->image=image;
    self->ivar=ivar;
    self->a=a;

    self->obj=gmix_new(ngauss);

    int npars=2*ngauss+4;

    self->burnin_chain = mca_chain_new(nwalkers,burn_per_walker,npars);
    self->chain = mca_chain_new(nwalkers,steps_per_walker,npars);

    self->stats = mca_stats_new(npars);

    if (psf) {
        self->psf=psf;
        int ntot=ngauss*psf->size;
        self->conv=gmix_new(ntot);
    }
    return self;
}
struct fitter_ce *fitter_ce_free(struct fitter_ce *self)
{
    if (self) {
        self->obj=gmix_free(self->obj);
        self->conv=gmix_free(self->conv);

        self->burnin_chain=mca_chain_free(self->burnin_chain);
        self->chain=mca_chain_free(self->chain);

        self->stats=mca_stats_free(self->stats);

        free(self);
    }
    return NULL;
}
// read the bound info into the object
int object_bound_read(struct object *self, FILE* fptr)
{
    int nread=
        fscanf(stdin,"%lu %lu %lf %lf %lu %lu %lu %lu",
                &self->orow,&self->ocol,
                &self->rowguess, &self->colguess,
                &self->mask.rowmin,&self->mask.rowmax,
                &self->mask.colmin,&self->mask.colmax);
    return (nread==8);
}

void coellip_g2e(const double *parsin, double *parsout, int npars)
{
    struct shape sh = {0};

    memcpy(parsout, parsin, npars*sizeof(double));
    shape_set_g(&sh, parsout[2], parsout[3]);

    parsout[2] = sh.e1;
    parsout[3] = sh.e2;
}


double coellip_lnprob(const double *pars, 
              size_t npars, 
              const void *void_data)
{
    double lnp=0;
    int flags=0;

    struct fitter_ce *data=(struct fitter_ce*) void_data;
    const struct gmix *gmix=NULL;


    // use this if g1,g2 are vars
    /*
    // stack allocated
    double *epars=alloca(npars*sizeof(double));
    coellip_g2e(pars, epars, npars);


    if (!gmix_fill_coellip(data->obj, pars, npars) ) {
        exit(EXIT_FAILURE);
    }

    if (data->psf) {
        if (!gmix_fill_convolve(data->conv, data->obj, data->psf)) {
            exit(EXIT_FAILURE);
        }
        gmix=data->conv;
    } else {
        gmix=data->obj;
    }

    // flags are only set for conditions we want to
    // propagate back through the likelihood
    lnp=gmix_image_loglike(data->image,
                           gmix,
                           data->ivar,
                           &flags);

    return lnp;
}

/*
double gen_lnprob(const double *pars, 
              size_t npars, 
              const void *void_data)
{
    double lnp=0;
    int flags=0;

    struct fit_data *data=(struct fit_data*) void_data;

    const struct gmix *gmix=NULL;

    if (data->par_type == GMIX_COELLIP) {
        if (!gmix_fill_coellip(data->obj, pars, npars) ) {
            exit(EXIT_FAILURE);
        }
    } else if (data->par_type == GMIX_DEV) {
        if (!gmix_fill_dev(data->obj, pars, npars) ) {
            exit(EXIT_FAILURE);
        }
    } else if (data->par_type == GMIX_EXP) {
        if (!gmix_fill_dev(data->obj, pars, npars) ) {
            exit(EXIT_FAILURE);
        }
    } else {
        wlog("error: expected par_type of approx dev,exp or coellip "
                "but got %d. %s: %d\n",
                data->par_type,__FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }


    if (data->psf) {
        if (!gmix_fill_convolve(data->conv, data->obj, data->psf)) {
            exit(EXIT_FAILURE);
        }
        gmix=data->conv;
    } else {
        gmix=data->obj;
    }

    // flags are only set for conditions we want to
    // propagate back through the likelihood
    lnp=gmix_image_loglike(data->image,
                           gmix,
                           data->ivar,
                           &flags);

    return lnp;
}

*/

struct am do_admom(struct image *image,
                   double rowguess, double colguess,
                   double Tguess, double skysig)
{
    struct am am = {{0}};

    am.nsigma = 4;
    am.maxiter = 100;
    am.shiftmax = 5;
    am.sky=0.0;
    am.skysig = skysig;

    fprintf(stderr,"row0: %lu col0: %lu\n",image->row0,image->col0);
    gauss2_set(&am.guess,
            1., rowguess, colguess, 
            Tguess/2., 0.0, Tguess/2.);

    admom(&am, image);

    return am;

}
void fill_coellip_guess(
        double row, double col,double T, 
        double counts,
        double *centers, double *widths, int npars)
{
    int ngauss=(npars-4)/2;
    centers[0]=row;
    centers[1]=col;
    centers[2]=0.;
    centers[3]=0.;
    widths[0]=1;
    widths[1]=1;
    widths[2]=.05;
    widths[3]=.05;

    // this is arbitrary
    //double T=4;
    if (ngauss==1) {
        centers[4] = T;
        centers[5] = counts;

        widths[4] = 0.1*centers[4];
        widths[5] = 0.1*centers[5];
    } else if (ngauss==2) {
        centers[4] = T;
        centers[5] = T*3./2.;  // arbitrary
        centers[6] = 0.55*counts;
        centers[7] = 0.45*counts;

        widths[4] = 0.1*centers[4];
        widths[5] = 0.1*centers[5];
        widths[6] = 0.1*centers[6];
        widths[7] = 0.1*centers[7];
    } else if (ngauss==3) {
        // dev
        // 2.12
        // 34.4
        // 340
        //1.05
        //1.66
        //2.04
        // exp guess
        centers[4] = T*4.e-5;
        centers[5] = T*0.5;
        centers[6] = T*1.9;

        centers[7] = 0.06*counts;
        centers[8] = 0.56*counts;
        centers[9] = 0.38*counts;
        /*
        Tsum = (2.12+34.4+340.0)
        psum = (1.06 + 1.66 + 2.04)
        centers[4] = 2.12;
        centers[5] = 34.4;
        centers[6] = 340.;
        centers[7] = 1.05*counts/psum;
        centers[8] = 1.66*counts/psum;
        centers[9] = 2.04*counts/psum;
        */

        widths[4] = 0.1*centers[4];
        widths[5] = 0.1*centers[5];
        widths[6] = 0.1*centers[6];
        widths[7] = 0.1*centers[7];
        widths[8] = 0.1*centers[8];
        widths[9] = 0.1*centers[9];

    }
}


/* second  object we use previous finish for start */
/*
void do_mca_run_uselast(struct fitter_ce *fitter, struct object *obj)
{

    // need to set center but otherwise use last
    size_t nwalkers=MCA_CHAIN_NWALKERS(fitter->chain);
    size_t nsteps=MCA_CHAIN_WNSTEPS(fitter->chain);
    for (size_t iwalker=0; iwalker<nwalkers; iwalker++) {
        double *pars=MCA_CHAIN_WPARS(fitter->chain, iwalker, (nsteps-1));
        pars[0] = obj->rowguess*(1.+0.01*2.*(drand48()-0.5));
        pars[1] = obj->colguess*(1.+0.01*2.*(drand48()-0.5));
    }

    mca_run(fitter->burnin_chain,fitter->a,
            fitter->chain, &coellip_lnprob,
            fitter);

    mca_run(fitter->chain,fitter->a,
            fitter->burnin_chain, &coellip_lnprob,
            fitter);


    mca_chain_stats_fill(fitter->stats, fitter->chain);


}
*/


/* first object we get a start from admom */
void do_mca_run_scratch(struct fitter_ce *fitter, struct object *obj)
{
    int npars=MCA_CHAIN_NPARS(fitter->chain);
    int nwalkers=MCA_CHAIN_NWALKERS(fitter->chain);

    // stack allocated
    double *centers=alloca(npars*sizeof(double));
    double *widths=alloca(npars*sizeof(double));

    // get local image coords
    //double rowguess=obj->rowguess-obj->mask.rowmin;
    //double colguess=obj->colguess-obj->mask.colmin;

    //fprintf(stderr,"im[31,32]: %.16g\n", IM_GET(fitter->image,31,32));
    //exit(1);

    // argh, this wants an unmasked image
    double Tstart=10.;
    struct am am = do_admom(fitter->image,
                            obj->rowguess, obj->colguess,
                            Tstart, sqrt(1./fitter->ivar));

    admom_print(&am, stderr);
    /*
    wlog("Tadmom:    %.16g\n", am.wt.irr+am.wt.icc);
    wlog("s/n admom: %.16g\n", am.s2n);
    */
    if (am.flags != 0) {
        wlog("admom flags: %d\n", am.flags);
        exit(EXIT_FAILURE);
    }
    double counts=image_get_counts(fitter->image);
    fill_coellip_guess(
            am.wt.row, am.wt.col,
            am.wt.irr+am.wt.icc,
            counts,
            centers,widths,npars);

    struct mca_chain *start=gmix_mcmc_make_guess_coellip(
            centers,
            widths,
            npars,
            nwalkers);

    mca_run(fitter->burnin_chain,fitter->a,
            start, &coellip_lnprob,
            fitter);

    mca_run(fitter->chain,fitter->a,
            fitter->burnin_chain, &coellip_lnprob,
            fitter);

    mca_chain_stats_fill(fitter->stats, fitter->chain);

    // do extra times on scratch run
    size_t nsteps=MCA_CHAIN_NSTEPS(fitter->chain);
    double e1mean_old = MCA_STATS_MEAN(fitter->stats, 2);
    double e2mean_old = MCA_STATS_MEAN(fitter->stats, 3);
    double Tmean_old = MCA_STATS_MEAN(fitter->stats, 4);

    double e1sum=e1mean_old*nsteps;
    double e2sum=e2mean_old*nsteps;
    double Tsum=Tmean_old*nsteps;

    size_t n=nsteps;

    //double etol=1.e-3;
    //double Ttol=0.01;
    double etol=1.e-3;
    double Ttol=0.01;
    int i=1;
    while (1) {
        //wlog(".");
        mca_run(fitter->chain,fitter->a,
                fitter->chain, &coellip_lnprob,
                fitter);
        mca_chain_stats_fill(fitter->stats, fitter->chain);
        double tmean1 = MCA_STATS_MEAN(fitter->stats, 2);
        double tmean2 = MCA_STATS_MEAN(fitter->stats, 3);
        // first T
        double tmeanT = MCA_STATS_MEAN(fitter->stats, 4);

        e1sum += tmean1*nsteps;
        e2sum += tmean2*nsteps;
        Tsum += tmeanT*nsteps;

        n+=nsteps;
        double e1mean = e1sum/n;
        double e2mean = e2sum/n;
        double Tmean = Tsum/n;

        double e1diff=fabs( e1mean-e1mean_old );
        double e2diff=fabs( e2mean-e2mean_old );
        double Tfdiff=fabs( Tmean/Tmean_old - 1.);

        wlog("%d mean %.6g (%.6g) %.6g (%.6g) %.6g (%.6g)\n", 
                i, e1mean, e1diff, e2mean, e2diff, Tmean, Tfdiff);

        if (e1diff < etol && e2diff < etol && Tfdiff < Ttol) {
            break;
        }
        e1mean_old=e1mean;
        e2mean_old=e2mean;
        Tmean_old=Tmean;
        i+=1;
    }

    mca_chain_stats_fill(fitter->stats, fitter->chain);

    start=mca_chain_free(start);

}
void process_object(struct fitters_ce *fitters,
                    struct object *obj)
{
    struct fitter_ce *psf_fitter=fitters->psf_fitter;
    struct fitter_ce *fitter=fitters->fitter;

    image_add_mask(fitter->image, &obj->mask);
    double counts=image_get_counts(fitter->image);
    fprintf(stderr,"image sub counts: %.16g\n", counts);

    // note both psf_fitter and fitter have pointers to psf
    // image, both will see the update
    image_add_mask(psf_fitter->image, &obj->mask);

    // see if it is a mask problem
    //psf_fitter->image=image_new_copy(psf_fitter->image);

    //exit(1);

    // this updates psf_fitter->obj, pointed to by
    // fitter->psf
    fprintf(stderr,"doing psf\n");
    do_mca_run_scratch(psf_fitter, obj);

    mca_stats_write_brief(psf_fitter->stats,stderr);

    mca_chain_write_file(psf_fitter->burnin_chain, "tmp/psf-burn-chain.dat");
    mca_chain_write_file(psf_fitter->chain, "tmp/psf-chain.dat");

    image_write_file(psf_fitter->image, "tmp/psfim.dat");
    struct image *psf_fit_im=gmix_image_new(
            psf_fitter->obj, 
            IM_NROWS(psf_fitter->image),
            IM_NCOLS(psf_fitter->image),
            16);
    image_write_file(psf_fit_im, "tmp/psfim-fit.dat");
    psf_fit_im=image_free(psf_fit_im);

    // will now use the psf
    fprintf(stderr,"doing convolved object\n");
    do_mca_run_scratch(fitter, obj);
    mca_stats_write_brief(fitter->stats,stderr);


    image_write_file(fitter->image, "tmp/im.dat");

    struct image *fit_im=gmix_image_new(
            fitter->conv, 
            IM_NROWS(fitter->image),
            IM_NCOLS(fitter->image),
            16);
    image_write_file(fit_im, "tmp/im-fit.dat");
    fit_im=image_free(fit_im);


    mca_chain_write_file(fitter->burnin_chain, "tmp/burn-chain.dat");
    mca_chain_write_file(fitter->chain, "tmp/chain.dat");

}


// hard wired for now
struct fitters_ce *fitters_new(
        struct config *config,
        struct image *image, 
        struct image *psf)
{

    fprintf(stderr,
            "a:          %.2lf\n"
            "psf:\n"
            "  model:    %s\n"
            "  ngauss:   %d\n"
            "  nwalkers: %d\n"
            "  burn:     %d\n"
            "  steps:    %d\n"
            "obj\n"
            "  model:    %s\n"
            "  ngauss:   %d\n"
            "  nwalkers: %d\n"
            "  burn:     %d\n"
            "  steps:    %d\n",
            config->a,
            config->psf_model,
            config->ngauss_psf,
            config->nwalkers_psf,
            config->burn_per_walker_psf,
            config->steps_per_walker_psf,
            config->obj_model,
            config->ngauss,
            config->nwalkers,
            config->burn_per_walker,
            config->steps_per_walker);


    struct fitters_ce *self=calloc(1,sizeof(struct fitters_ce));
    if (!self) {
        fprintf(stderr,"error: could not make struct fitters_ce\n");
        exit(EXIT_FAILURE);
    }

    double ivar = 1./(config->skysig*config->skysig);
    double psf_ivar = 1./(config->skysig_psf*config->skysig_psf);

    self->psf_fitter=fitter_ce_new(
            psf, psf_ivar, config->a,
            config->nwalkers_psf, 
            config->burn_per_walker_psf, 
            config->steps_per_walker_psf,
            config->ngauss_psf, NULL);
    self->fitter=fitter_ce_new(
            image, ivar, config->a,
            config->nwalkers, 
            config->burn_per_walker, 
            config->steps_per_walker,
            config->ngauss, 
            self->psf_fitter->obj); // const reference

    return self;
}

void read_config(struct config *config, const char *fname)
{
    enum cfg_status status=0;
    struct cfg *cfg=cfg_read(fname, &status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"error parsing config "
                       "'%s': %s\n", fname, cfg_status_string(status));
        exit(EXIT_FAILURE);
    }

    config->a = cfg_get_double(cfg, "a", &status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting a as double: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }

    config->obj_model = cfg_get_string(cfg, "model",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting model as string: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }
    config->psf_model = cfg_get_string(cfg, "model_psf",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting model_psf as string: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }


    if (0 == strcmp(config->obj_model,"coellip")) {
        config->obj_type = GMIX_COELLIP;
        config->ngauss = (int)cfg_get_long(cfg, "ngauss",&status);
        if (status != CFG_SUCCESS) {
            fprintf(stderr,"config error getting ngauss as long: %s\n", cfg_status_string(status));
            exit(EXIT_FAILURE);
        }
    } else if (0 == strcmp(config->obj_model,"gdev")) {
    } else {
        fprintf(stderr,"error: only coellip for now\n");
        exit(EXIT_FAILURE);
    }
    if (0 == strcmp(config->psf_model,"coellip")) {
        config->psf_type = GMIX_COELLIP;
        config->ngauss_psf = (int)cfg_get_long(cfg, "ngauss_psf",&status);
        if (status != CFG_SUCCESS) {
            fprintf(stderr,"config error getting ngauss_psf as long: %s\n", cfg_status_string(status));
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr,"error: only coellip for now\n");
        exit(EXIT_FAILURE);
    }

    config->nwalkers = (int)cfg_get_long(cfg, "nwalkers",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting nwalkers as long: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }
    config->burn_per_walker = (int)cfg_get_long(cfg, "burn_per_walker",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting burn_per_walker as long: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }
    config->steps_per_walker = (int)cfg_get_long(cfg, "steps_per_walker",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting steps_per_walker as long: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }

    config->nwalkers_psf = (int)cfg_get_long(cfg, "nwalkers_psf",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting nwalkers_psf as long: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }
    config->burn_per_walker_psf = (int)cfg_get_long(cfg, "burn_per_walker_psf",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting burn_per_walker_psf as long: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }
    config->steps_per_walker_psf = (int)cfg_get_long(cfg, "steps_per_walker_psf",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting steps_per_walker_psf as long: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }

    config->skysig = cfg_get_double(cfg, "skysig",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting skysig as double: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }
    config->skysig_psf = cfg_get_double(cfg, "skysig_psf",&status);
    if (status != CFG_SUCCESS) {
        fprintf(stderr,"config error getting skysig_psf as double: %s\n", cfg_status_string(status));
        exit(EXIT_FAILURE);
    }




    cfg=cfg_free(cfg);
}
int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr,"gmix-galsim config image psf < objlist > output \n");
        exit(EXIT_FAILURE);
    }

    printf("ADAPT TO NEW gmix_image_loglike that works with center in sub image\n");
    exit(0);

    if (0 != system("mkdir -p tmp")) {
        fprintf(stderr,"could not make ./tmp");
        exit(1);
    }

    struct config config={0};

    const char *config_file=argv[1];
    const char *image_file=argv[2];
    const char *psf_file=argv[3];

    read_config(&config, config_file);

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);

    struct image *image=image_read_fits(image_file,0);
    struct image *psf=image_read_fits(psf_file,0);

    // need some noise
    image_add_randn(psf, config.skysig_psf);

    // from mike, need to put in a config file
    struct fitters_ce *fitters=fitters_new(&config,image,psf);

    struct object object={0};

    // head of stdout will be the fit type of obj and psf
    printf("coellip coellip\n"); 
    // only do psf on first object
    while (object_bound_read(&object,stdin)) {

        process_object(fitters,&object);

        fprintf(stdout,"%lu %lu ",object.orow,object.ocol);
        mca_stats_write_flat(fitters->fitter->stats,stdout);
        mca_stats_write_flat(fitters->psf_fitter->stats,stdout);
        printf("\n");
    }
                     
    psf=image_free(psf);
    image=image_free(image);

}
