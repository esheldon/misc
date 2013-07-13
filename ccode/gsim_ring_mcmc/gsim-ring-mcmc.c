#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
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
#include "object.h"
#include "result.h"

// make an object list
struct obs_list *make_obs_list(const struct image *image,
                               const struct image *weight,
                               const struct image *psf_image,
                               long psf_ngauss,
                               long *flags)

{
    struct obs_list *self=obs_list_new(1);
    struct jacobian jacob = {0};

    jacobian_set_identity(&jacob);

    obs_fill(&self->data[0],
             image,
             weight,
             psf_image,
             &jacob,
             psf_ngauss,
             flags);
    if (*flags != 0) {
        self=obs_list_free(self);
    }
    return self;
}

struct ring_image_pair *get_image_pair(struct object *obj, long *flags)
{
    struct ring_pair *rpair=NULL;
    struct ring_image_pair *impair=NULL;

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
                          flags);
    if (*flags != 0) {
        goto _get_image_pair_bail;
    }

    ring_pair_print(rpair,stdout);

    impair = ring_image_pair_new(rpair, flags);

    if (*flags != 0) {
        goto _get_image_pair_bail;
    }

    printf("skysig1: %g  skysig2: %g psf_skysig: %g\n",
           impair->skysig1, impair->skysig2, impair->psf_skysig);

_get_image_pair_bail:
    rpair = ring_pair_free(rpair);
    if (*flags != 0) {
        impair = ring_image_pair_free(impair);
    }

    return impair;
}

void process_obs_list(struct gmix_mcmc_config *conf,
                      struct obs_list *obs_list,
                      struct result *res)
{
}

void process_pair(struct gmix_mcmc_config *conf,
                  struct object *obj,
                  struct result *res1,
                  struct result *res2,
                  long *flags)
{
    struct obs_list *obs_list=NULL;
    struct ring_image_pair *impair = get_image_pair(obj, flags);
    if (*flags != 0) {
        goto _process_object_bail;
    }

    obs_list = make_obs_list(impair->im1,
                             impair->wt1,
                             impair->psf_image,
                             conf->psf_ngauss,
                             flags);
    if (*flags != 0) {
        goto _process_object_bail;
    }

    obs_list = obs_list_free(obs_list);

    obs_list = make_obs_list(impair->im2,
                             impair->wt2,
                             impair->psf_image,
                             conf->psf_ngauss,
                             flags);

_process_object_bail:
    obs_list = obs_list_free(obs_list);
    impair = ring_image_pair_free(impair);

    return;
}

static struct gmix_mcmc_config *load_config(const char *name)
{
    enum cfg_status cfg_stat=0;
    struct gmix_mcmc_config *conf=gmix_mcmc_config_read(name, &cfg_stat);
    if (cfg_stat!=0) {
        fprintf(stderr,"fatal error reading conf, exiting\n");
        exit(1);
    }
    gmix_mcmc_config_print(conf, stdout);

    return conf;
}

FILE *open_file(const char *name, const char *mode)
{
    FILE *stream = fileio_open_stream(name,mode);
    if (!stream) {
        exit(1);
    }
    return stream;
}



static struct prob_data_simple_gmix3_eta *
get_prob(struct gmix_mcmc_config *conf)
{

    struct prob_data_simple_gmix3_eta *prob=NULL;

    prob_data_simple_gmix3_eta_new(enum gmix_model model,
                                   long psf_ngauss,

                                   const struct dist_gauss *cen1_prior,
                                   const struct dist_gauss *cen2_prior,

                                   const struct dist_gmix3_eta *shape_prior,

                                   const struct dist_lognorm *T_prior,
                                   const struct dist_lognorm *counts_prior,
                                   long *flags)

}

void run_sim(struct gmix_mcmc_config *conf, FILE* input_stream, FILE* output_stream)
{
    struct object obj={{0}};
    struct result *res1=NULL, *res2=NULL;
    long flags=0;

    long nlines = fileio_count_lines(input_stream);
    rewind(input_stream);

    // always work in pairs
    res1=result_new(conf->nwalkers, conf->burnin, conf->nstep, conf->npars, conf->mca_a);
    res2=result_new(conf->nwalkers, conf->burnin, conf->nstep, conf->npars, conf->mca_a);

    fprintf(stderr,"processing %ld objects\n\n", nlines);

    for (long i=0; i<nlines; i++) {
        if (!object_read(&obj, input_stream)) {
            fprintf(stderr, "error reading object, aborting: %s: %d", __FILE__,__LINE__);
            exit(1);
        }

        object_print(&obj, stdout);
        process_pair(conf, &obj, res1, res2, &flags);
        if (flags != 0) {
            fprintf(stderr, "error processing ring pair, aborting: %s: %d", __FILE__,__LINE__);
            exit(1);
        }
    }

    res1=result_free(res1);
    res2=result_free(res2);
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("usage: %s config objlist output_file\n", argv[0]);
        exit(1);
    }

    struct gmix_mcmc_config *conf=load_config(argv[1]);
    FILE *input_stream = open_file(argv[2],"r");
    FILE *output_stream = open_file(argv[3],"w");

    printf("running sim\n");
    run_sim(conf, input_stream, output_stream);
    printf("finished running sim\n");

    conf = gmix_mcmc_config_free(conf);
    fclose(input_stream);
    fclose(output_stream);
    return 0;
}
