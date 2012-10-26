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

#include "fitsio.h"

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
    shape_set_g1g2(&sh, parsout[2], parsout[3]);

    parsout[2] = sh.e1;
    parsout[3] = sh.e2;
}


double coellip_lnprob(const double *pars, 
              size_t npars, 
              const void *void_data)
{
    double lnp=0;
    int flags=0;

    // stack allocated
    double *epars=alloca(npars*sizeof(double));

    // gmix coellip and the like code work in e1,e2 space
    coellip_g2e(pars, epars, npars);

    struct fitter_ce *data=(struct fitter_ce*) void_data;

    const struct gmix *gmix=NULL;

    if (!gmix_fill_coellip(data->obj, epars, npars) ) {
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
    } else if (data->par_type == GMIX_APPROX_DEV) {
        if (!gmix_fill_dev(data->obj, pars, npars) ) {
            exit(EXIT_FAILURE);
        }
    } else if (data->par_type == GMIX_APPROX_EXP) {
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
    gauss_set(&am.guess,
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
        centers[6] = 0.6*counts;
        centers[7] = 0.4*counts;

        widths[4] = 0.1*centers[4];
        widths[5] = 0.1*centers[5];
        widths[6] = 0.1*centers[6];
        widths[7] = 0.1*centers[7];
    } else if (ngauss==3) {
        // exp guess
        centers[4] = T*4.e-5;
        centers[5] = T*0.5;
        centers[6] = T*1.9;

        centers[7] = 0.06*counts;
        centers[8] = 0.56*counts;
        centers[9] = 0.38*counts;

        widths[4] = 0.1*centers[4];
        widths[5] = 0.1*centers[5];
        widths[6] = 0.1*centers[6];
        widths[7] = 0.1*centers[7];
        widths[8] = 0.1*centers[8];
        widths[9] = 0.1*centers[9];

    }
}


void do_mca_run(struct fitter_ce *fitter, struct object *obj)
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
    fill_coellip_guess(
            am.wt.row, am.wt.col,
            am.wt.irr+am.wt.icc,
            IM_COUNTS(fitter->image),
            centers,widths,npars);

    struct mca_chain *start=gmix_mcmc_make_guess_coellip(
            centers,
            widths,
            npars,
            nwalkers);
    /*
    wlog("counts: %lf\n", IM_COUNTS(fitter->image));
    wlog("start\n");
    mca_chain_write(start,stderr);
    */

    mca_run(fitter->burnin_chain,fitter->a,
            start, &coellip_lnprob,
            fitter);

    mca_run(fitter->chain,fitter->a,
            fitter->burnin_chain, &coellip_lnprob,
            fitter);


    mca_chain_stats_fill(fitter->stats, fitter->chain);

    start=mca_chain_free(start);

}
void process_object(struct fitters_ce *fitters,
                    struct object *obj, 
                    int dopsf)
{
    int update_counts=1;
    struct fitter_ce *psf_fitter=fitters->psf_fitter;
    struct fitter_ce *fitter=fitters->fitter;

    image_add_mask(fitter->image, &obj->mask, update_counts);
    fprintf(stderr,"image sub counts: %.16g\n", IM_COUNTS(fitter->image));

    if (dopsf) {
        // note both psf_fitter and fitter have pointers to psf
        // image, both will see the update
        image_add_mask(psf_fitter->image, &obj->mask, update_counts);

        // see if it is a mask problem
        //psf_fitter->image=image_newcopy(psf_fitter->image);

        //exit(1);

        // this updates psf_fitter->obj, pointed to by
        // fitter->psf
        fprintf(stderr,"doing psf\n");
        do_mca_run(psf_fitter, obj);
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

    }
    // will now use the psf
    fprintf(stderr,"doing convolved object\n");
    do_mca_run(fitter, obj);
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
        struct image *image, double ivar,
        struct image *psf, double psf_ivar)
{
    double a=2;
    // put in a config!
    int ngauss=1;
    int ngauss_psf=2;

    // super high s/n takes a lot more burn-in
    int psf_nwalkers=20;
    //int psf_burn_per_walker=200;
    int psf_burn_per_walker=20000;
    int psf_steps_per_walker=200;


    int nwalkers=20;
    int burn_per_walker=200;
    int steps_per_walker=200;

    fprintf(stderr,
            "psf:\n"
            "  nwalkers: %d\n"
            "  burn:     %d\n"
            "  steps:    %d\n"
            "obj\n"
            "  nwalkers: %d\n"
            "  burn:     %d\n"
            "  steps:    %d\n",
            psf_nwalkers,
            psf_burn_per_walker,
            psf_steps_per_walker,
            nwalkers,
            burn_per_walker,
            steps_per_walker);



    struct fitters_ce *self=calloc(1,sizeof(struct fitters_ce));
    if (!self) {
        fprintf(stderr,"error: could not make struct fitters_ce\n");
        exit(EXIT_FAILURE);
    }

    self->psf_fitter=fitter_ce_new(
            psf, psf_ivar, a,
            psf_nwalkers, psf_burn_per_walker, psf_steps_per_walker,
            ngauss_psf, NULL);
    self->fitter=fitter_ce_new(
            image, ivar, a,
            nwalkers, burn_per_walker, steps_per_walker,
            ngauss, 
            self->psf_fitter->obj); // const reference

    return self;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr,"gmix-galsim image psf < objlist > output \n");
        exit(EXIT_FAILURE);
    }

    if (0 != system("mkdir -p tmp")) {
        fprintf(stderr,"could not make ./tmp");
        exit(1);
    }


    const char *image_file=argv[1];
    const char *psf_file=argv[2];

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);

    struct image *image=image_read_fits(image_file,0);
    struct image *psf=image_read_fits(psf_file,0);

    // need some noise in the psf image
    double psf_skysig=1.e-4; // gives admom s/n ~1300
    double psf_ivar=1/(psf_skysig*psf_skysig);
    image_add_randn(psf, psf_skysig);

    // from mike, need to put in a config file
    double skysig=281.;
    double ivar=1/(skysig*skysig);
    struct fitters_ce *fitters=fitters_new(image,ivar,psf,psf_ivar);

    struct object object={0};

    // only do psf on first object
    int dopsf=1;
    while (object_bound_read(&object,stdin)) {

        process_object(fitters,&object,dopsf);
        dopsf=0;

        fprintf(stdout,"%lu %lu ",object.orow,object.ocol);
        mca_stats_write_flat(fitters->fitter->stats,stdout);
        mca_stats_write_flat(fitters->psf_fitter->stats,stdout);
        printf("\n");
    }
                     
    psf=image_free(psf);
    image=image_free(image);

}
