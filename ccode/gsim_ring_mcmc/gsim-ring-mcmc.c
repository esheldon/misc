#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
#include "gmix_em.h"
#include "gsim_ring.h"
#include "shape.h"
#include "image.h"
#include "obs.h"
#include "time.h"
#include "fileio.h"

#include "mca.h"

#include "config.h"
#include "result.h"
#include "gmix_mcmc_config.h"
#include "gmix_mcmc.h"
#include "object.h"
#include "result.h"

#include "randn.h"

// make an object list
struct obs_list *make_obs_list(const struct image *image,
                               const struct image *weight,
                               const struct image *psf_image,
                               long psf_ngauss,
                               double row,
                               double col,
                               long *flags)

{
    struct obs_list *self=obs_list_new(1);
    struct jacobian jacob = {0};

    jacobian_set_identity(&jacob);
    jacobian_set_cen(&jacob, row, col);

    obs_fill(&self->data[0],
             image,
             weight,
             psf_image,
             &jacob,
             psf_ngauss,
             flags);
    //jacobian_print(&self->data[0].jacob,stderr);

    if (*flags != 0) {
        self=obs_list_free(self);
    }
    return self;
}

// note row,col returned are the "center start", before offsetting,
// so we can properly use it in the prior
static
struct ring_image_pair *get_image_pair(struct object *obj,
                                       double *row, double *col,// for guesses
                                       double *T, double *counts)
{
    struct ring_pair *rpair=NULL;
    struct ring_image_pair *impair=NULL;
    double psf_row=0, psf_col=0;
    long flags=0;

    rpair = ring_pair_new(obj->model,
                          obj->pars,
                          obj->npars,
                          obj->psf_model,
                          obj->psf_pars,
                          obj->psf_npars,
                          &obj->shear,
                          obj->s2n,
                          obj->cen1_offset,
                          obj->cen2_offset,
                          &flags);
    if (flags != 0) {
        goto _get_image_pair_bail;
    }

    ring_pair_print(rpair,stderr);

    impair = ring_image_pair_new(rpair, row, col, &psf_row, &psf_col, &flags);
    //image_view(impair->im1, "-m");
    //image_view(impair->im2, "-m");

    if (flags != 0) {
        goto _get_image_pair_bail;
    }

    fprintf(stderr,"skysig1: %g  skysig2: %g psf_skysig: %g\n",
           impair->skysig1, impair->skysig2, impair->psf_skysig);

    *T=gmix_get_T(rpair->gmix1);
    *counts=gmix_get_psum(rpair->gmix1);

_get_image_pair_bail:
    rpair = ring_pair_free(rpair);
    if (flags != 0) {
        impair = ring_image_pair_free(impair);
        fprintf(stderr,"failed to make image pair, aborting: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    return impair;
}

void print_one(const struct gmix_mcmc *self,
               const struct result *res)
{
    mca_stats_write_brief(self->chain_data.stats, stdout);
    mca_stats_write_flat(self->chain_data.stats, stdout);
    result_print(res, stdout);

    fprintf(stdout,"\n");
}

static void fill_psf_gmix_guess(struct gmix *self, double row, double col, double counts)
{
    long flags=0;
    double frac = counts/self->size;
    for (long i=0; i<self->size; i++) {
        struct gauss2 *g=&self->data[i];
        gauss2_set(g,
                   frac + 0.1*srandu(),
                   row + srandu(),
                   col + srandu(),
                   2.0 + srandu(), 
                   0.0 + 0.1*srandu(), 
                   2.0 + srandu(),
                   &flags);
    }
}

//
// process all the PSFs in the set of observations
// need to move this code into gmix_em
//

static double get_em_sky_to_add(double im_min)
{
    return 0.001 - im_min;
}
static void process_psfs(struct gmix_mcmc *self)
{
    long n_retry = 100, try=0;
    struct gmix_em gmix_em={0};
    double min=0, max=0;

    gmix_em.maxiter=self->conf.em_maxiter;
    gmix_em.tol=self->conf.em_tol;

    const struct obs_list *obs_list=self->obs_list;
    for (long i=0; i<obs_list->size; i++) {
        const struct obs *obs=&obs_list->data[i];
        const struct image *psf_image = obs->psf_image;
        struct gmix *psf_gmix = obs->psf_gmix;

        // counts should be pre-sky addition
        double counts = image_get_counts(psf_image);

        struct image *psf_image_sky = image_new_copy(psf_image);
        image_get_minmax(psf_image, &min, &max);

        double sky_to_add = get_em_sky_to_add(min);
        image_add_scalar(psf_image_sky, sky_to_add);
        IM_SET_SKY(psf_image_sky, sky_to_add);

        double row=(IM_NROWS(psf_image)-1.0)/2.0;
        double col=(IM_NCOLS(psf_image)-1.0)/2.0;

        for (try=0; try<n_retry; try++) {
            fill_psf_gmix_guess(psf_gmix, row, col, counts);
            gmix_em_run(&gmix_em, psf_image_sky, psf_gmix);
            if (gmix_em.flags == 0) {
                break;
            }
        }

        if (try==n_retry) {
            // can later use or not use psf
            fprintf(stderr, "error processing psf failed %ld times, aborting: %s: %d", 
                    n_retry, __FILE__,__LINE__);
            exit(1);
        }

        psf_image_sky=image_free(psf_image_sky);
    }

}
void process_one(struct gmix_mcmc *self,
                 const struct obs_list *obs_list,
                 double row,
                 double col,
                 double T,
                 double counts,
                 struct result *res,
                 long *flags)
{
    gmix_mcmc_set_obs_list(self, obs_list);

    process_psfs(self);

    gmix_mcmc_run(self, row, col, T, counts, flags);
    if (*flags != 0) {
        goto _process_one_bail;
    }

    mca_chain_stats_fill(self->chain_data.stats, self->chain_data.chain);
    result_calc(res, &self->chain_data);
#if 1
    mca_chain_plot(self->chain_data.burnin_chain,"");
    mca_chain_plot(self->chain_data.chain,"");
#endif
_process_one_bail:
    return;
}

void process_pair(struct gmix_mcmc *self,
                  struct object *obj,
                  struct result *res1,
                  struct result *res2)
{

    long flags=0;
    struct obs_list *obs_list=NULL;
    double row=0, col=0, T=0, counts=0;
    double row_guess=0, col_guess=0;

    struct ring_image_pair *impair = get_image_pair(obj, 
                                                    &row,
                                                    &col,
                                                    &T,
                                                    &counts);

    obs_list = make_obs_list(impair->im1,
                             impair->wt1,
                             impair->psf_image,
                             self->conf.psf_ngauss,
                             row,
                             col,
                             &flags);
    if (flags != 0) {
        goto _process_pair_bail;
    }

    fprintf(stderr,"running image 1 mcmc\n");
    process_one(self,
                obs_list,
                row_guess, // in jacob coord system
                col_guess,
                T,counts,
                res1,
                &flags);

    if (flags != 0) {
        goto _process_pair_bail;
    }
    print_one(self, res1);

    obs_list = obs_list_free(obs_list);
    obs_list = make_obs_list(impair->im2,
                             impair->wt2,
                             impair->psf_image,
                             self->conf.psf_ngauss,
                             row,
                             col,
                             &flags);
    if (flags != 0) {
        goto _process_pair_bail;
    }


    fprintf(stderr,"running image 2 mcmc\n");
    process_one(self,
                obs_list,
                row_guess, // in jacob coord system
                col_guess,
                T,
                counts,
                res2,
                &flags);

    if (flags != 0) {
        goto _process_pair_bail;
    }
    print_one(self, res2);

_process_pair_bail:
    obs_list = obs_list_free(obs_list);
    impair = ring_image_pair_free(impair);
    if (flags != 0) {
        fprintf(stderr, "error processing pair, aborting: %s: %d", __FILE__,__LINE__);
        exit(1);
    }

    return;
}

void run_sim(struct gmix_mcmc_config *conf, FILE* input_stream, FILE* output_stream)
{
    struct object obj={{0}};
    struct result res1 = {0}, res2={0};
    struct gmix_mcmc *gmix_mcmc=NULL;

    long flags=0;

    gmix_mcmc = gmix_mcmc_new(conf, &flags);
    if (flags != 0) {
        goto _run_sim_bail;
    }

    long nlines = fileio_count_lines(input_stream);
    rewind(input_stream);

    // always work in pairs

    fprintf(stderr,"processing %ld objects\n\n", nlines);

    for (long i=0; i<nlines; i++) {
        if (!object_read(&obj, input_stream)) {
            fprintf(stderr, "error reading object, aborting: %s: %d", __FILE__,__LINE__);
            exit(1);
        }

        object_print(&obj, stderr);
        process_pair(gmix_mcmc, &obj, &res1, &res2);
    }
_run_sim_bail:
    gmix_mcmc = gmix_mcmc_free(gmix_mcmc);

}

static void load_config(struct gmix_mcmc_config *conf, const char *name)
{
    long flags=gmix_mcmc_config_load(conf, name);
    if (flags != 0) {
        fprintf(stderr,"fatal error reading conf, exiting\n");
        exit(1);
    }
    gmix_mcmc_config_print(conf, stderr);
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("usage: %s config objlist output_file\n", argv[0]);
        exit(1);
    }
    randn_seed();

    struct gmix_mcmc_config conf={0};
    load_config(&conf, argv[1]);
    FILE *input_stream = fileio_open_or_die(argv[2],"r");
    FILE *output_stream = fileio_open_or_die(argv[3],"w");

    printf("running sim\n");
    run_sim(&conf, input_stream, output_stream);
    printf("finished running sim\n");

    fclose(input_stream);
    fclose(output_stream);
    return 0;
}
