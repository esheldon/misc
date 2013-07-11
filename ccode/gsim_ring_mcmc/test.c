/*
   Only for testing loading of objects and creating image pairs.
*/
#include <stdlib.h>
#include <stdio.h>
#include "gmix.h"
#include "gsim_ring.h"
#include "shape.h"
#include "image.h"
#include "time.h"
#include "fileio.h"

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

    long flags=0;
    if (argc < 2) {
        printf("usage: test objlist\n");
        exit(1);
    }

    FILE *stream = open_file(argv[1]);
    if (!stream) {
        exit(1);
    }

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

    fclose(stream);
    return 0;
}
