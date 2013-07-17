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
#include "gmix_mcmc.h"
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

void process_pair(struct gmix_mcmc *self,
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
                             self->conf.psf_ngauss,
                             flags);
    if (*flags != 0) {
        goto _process_object_bail;
    }

    obs_list = obs_list_free(obs_list);

    obs_list = make_obs_list(impair->im2,
                             impair->wt2,
                             impair->psf_image,
                             self->conf.psf_ngauss,
                             flags);

_process_object_bail:
    obs_list = obs_list_free(obs_list);
    impair = ring_image_pair_free(impair);

    return;
}

static void load_config(struct gmix_mcmc_config *conf, const char *name)
{
    long flags=gmix_mcmc_config_load(conf, name);
    if (flags != 0) {
        fprintf(stderr,"fatal error reading conf, exiting\n");
        exit(1);
    }
    gmix_mcmc_config_print(conf, stdout);
}


void run_sim(struct gmix_mcmc_config *conf, FILE* input_stream, FILE* output_stream)
{
    struct object obj={{0}};
    struct result res1 = {{0}}, res2={{0}};
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

        object_print(&obj, stdout);
        process_pair(gmix_mcmc, &obj, &res1, &res2, &flags);
        if (flags != 0) {
            fprintf(stderr, "error processing ring pair, aborting: %s: %d", __FILE__,__LINE__);
            exit(1);
        }
    }
_run_sim_bail:
    gmix_mcmc = gmix_mcmc_free(gmix_mcmc);

}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("usage: %s config objlist output_file\n", argv[0]);
        exit(1);
    }
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
