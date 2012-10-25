/*

   This is a quick app to run on some high s/n galsim 
   images.

   Only coellip for now.

*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "image.h"
#include "gmix.h"
#include "gmix_image.h"
#include "gmix_image_fits.h"
#include "gmix_mcmc.h"
#include "mca.h"
#include "randn.h"

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

void _image_add_noise(struct image *image, double skysig)
{
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);

    size_t col=0, row=0;
    double *rowdata=NULL;

    for (row=0; row<nrows; row++) {
        rowdata=IM_ROW(image, row);
        for (col=0; col<ncols; col++) {
            (*rowdata) += skysig*randn();
            rowdata++;
        } // cols
    } // rows
}


double coellip_lnprob(const double *pars, 
              size_t npars, 
              const void *void_data)
{
    double lnp=0;
    int flags=0;

    struct fitter_ce *data=(struct fitter_ce*) void_data;

    const struct gmix *gmix=NULL;

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

    /*
    fprintf(stderr,"%.16g\n", lnp);
    gmix_print(gmix, stderr);
    */
    /*
    if (flags != 0) {
        fprintf(stderr,"flags: %d lnp: %.16g\n", flags, lnp);
    }
    */
    /*
    double cenp1 = (pars[0]-31.5)/3.;
    double cenp2 = (pars[1]-31.5)/3.;
    lnp += -0.5*(cenp1*cenp1 + cenp2*cenp2);
    */
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

void fill_coellip_guess(double row, double col,double counts,
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
    double T=4;
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
    double rowguess=obj->rowguess-obj->mask.rowmin;
    double colguess=obj->colguess-obj->mask.colmin;

    //fprintf(stderr,"im[31,32]: %.16g\n", IM_GET(fitter->image,31,32));
    //exit(1);

    fill_coellip_guess(rowguess, colguess,
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
    /*
    wlog("burnin\n");
    mca_chain_write_file(fitter->burnin_chain, "tchain.dat");
    exit(1);
    */

    mca_run(fitter->chain,fitter->a,
            fitter->burnin_chain, &coellip_lnprob,
            fitter);


    mca_chain_stats_fill(fitter->stats, fitter->chain);

    start=mca_chain_free(start);

}
void process_object(struct fitters_ce *fitters,
                    struct object *obj)
{
    int update_counts=1;
    struct fitter_ce *psf_fitter=fitters->psf_fitter;
    struct fitter_ce *fitter=fitters->fitter;

    image_add_mask(fitter->image, &obj->mask, update_counts);

    // note both psf_fitter and fitter have pointers to psf
    // image, both will see the update
    image_add_mask(psf_fitter->image, &obj->mask, update_counts);

    // see if it is a mask problem
    //psf_fitter->image=image_newcopy(psf_fitter->image);

    //image_write_file(psf_fitter->image, "tmpim.dat");
    //exit(1);

    // this updates psf_fitter->obj, pointed to by
    // fitter->psf
    fprintf(stderr,"doing psf\n");
    do_mca_run(psf_fitter, obj);
    mca_stats_write_brief(psf_fitter->stats,stderr);

    // will now use the psf
    fprintf(stderr,"doing convolved object\n");
    do_mca_run(fitter, obj);
    mca_stats_write_brief(fitter->stats,stderr);
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
    int nwalkers=20;
    int burn_per_walker=200;
    int steps_per_walker=200;


    struct fitters_ce *self=calloc(1,sizeof(struct fitters_ce));
    if (!self) {
        fprintf(stderr,"error: could not make struct fitters_ce\n");
        exit(EXIT_FAILURE);
    }

    self->psf_fitter=fitter_ce_new(
            psf, psf_ivar, a,
            nwalkers, burn_per_walker, steps_per_walker,
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

    const char *image_file=argv[1];
    const char *psf_file=argv[2];

    time_t tm;
    (void) time(&tm);
    srand48((long) tm);

    struct image *image=image_read_fits(image_file,0);
    struct image *psf=image_read_fits(psf_file,0);

    // need some noise in the psf image
    double psf_skysig=1.e-5;
    double psf_ivar=1/(psf_skysig*psf_skysig);
    _image_add_noise(psf, psf_skysig);

    // from mike, need to put in a config file
    double ivar=1/1.e6;
    struct fitters_ce *fitters=fitters_new(image,ivar,psf,psf_ivar);

    struct object object={0};

    while (object_bound_read(&object,stdin)) {
        /*
        object.mask.rowmin=object.rowguess-8;
        object.mask.rowmax=object.rowguess+8;
        object.mask.colmin=object.colguess-8;
        object.mask.colmax=object.colguess+8;
        */
        object_write(&object,stderr);
        fprintf(stderr,"%lu %lu\n",object.orow,object.ocol);

        process_object(fitters,&object);
        exit(1);

        fprintf(stdout,"%lu %lu ",object.orow,object.ocol);
        mca_stats_write_flat(fitters->fitter->stats,stdout);
        mca_stats_write_flat(fitters->psf_fitter->stats,stdout);
        printf("\n");
    }
                     
    psf=image_free(psf);
    image=image_free(image);

}
