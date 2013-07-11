#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
#include "gsim_ring.h"
#include "shape.h"
#include "image.h"
#include "time.h"
#include "fileio.h"

#include "mca.h"

#include "config.h"
#include "gmix_mcmc_config.h"
#include "object.h"

static FILE *open_file(const char *name)
{
    FILE *fobj=fopen(name,"r");
    if (!fobj) {
        fprintf(stderr,"error opening file: %s\n", name);
    }
    return fobj;
}

int main(int argc, char **argv)
{

    //long flags=0;
    enum cfg_status cfg_stat=0;

    if (argc < 3) {
        printf("usage: test config objlist\n");
        exit(1);
    }

    struct gmix_mcmc_config *conf=gmix_mcmc_config_read(argv[1], &cfg_stat);
    if (cfg_stat!=0) {
        fprintf(stderr,"fatal error reading conf, exiting\n");
        exit(1);
    }
    gmix_mcmc_config_print(conf, stdout);

    FILE *stream = open_file(argv[2]);
    if (!stream) {
        exit(1);
    }

    /*
    long nlines = fileio_count_lines(stream);
    rewind(stream);

    fprintf(stderr,"reading %ld objects\n\n", nlines);

    struct object obj={{0}};
    struct ring_image_pair *impair=NULL;
    struct ring_pair *rpair=NULL;
    for (long i=0; i<nlines; i++) {
        if (!object_read(&obj, stream)) {
            fprintf(stderr, "error reading object, aborting: %s: %d",
                    __FILE__,__LINE__);
            exit(1);
        }

        object_print(&obj, stdout);

        rpair = ring_pair_new(obj.model,
                              obj.pars,
                              obj.npars,
                              obj.psf_model,
                              obj.psf_pars,
                              obj.psf_npars,
                              &obj.shear,
                              obj.s2n,
                              obj.cen1_offset,
                              obj.cen2_offset,
                              &flags);
        if (flags != 0) {
            goto _loop_cleanup;
        }

        ring_pair_print(rpair,stdout);

        impair = ring_image_pair_new(rpair, &flags);

        if (flags != 0) {
            goto _loop_cleanup;
        }

        printf("skysig1: %g  skysig2: %g\n", impair->skysig1, impair->skysig2);

_loop_cleanup:
        rpair = ring_pair_free(rpair);
        impair = ring_image_pair_free(impair);
        if (flags != 0) {
            goto _bail;
        }
    }
_bail:

    */
    conf = gmix_mcmc_config_free(conf);
    fclose(stream);
    return 0;
}
