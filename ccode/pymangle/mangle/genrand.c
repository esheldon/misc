/*
 * generate random points within the mask and according
 * to the weight of each polygon.
 *
 * Points are first generated over the whole sky and then
 * checked against the mask.  If you know the boundary of
 * your mask ahead of time, it would be better to use
 * the genrand_range() function instead of genrand_allsky()
 *
 * output is
 *     ra dec polyid weight
 *
 */
#include <stdlib.h>
#include <stdio.h>

#include "mangle.h"
#include "defs.h"
#include "rand.h"

void do_genrand(struct MangleMask* mask, long nrand)
{
    int64 poly_id=0;
    double weight=0;
    struct Point pt;

    seed_random();
    while (nrand > 0) {
        genrand_allsky(&pt);
        if (!mangle_polyid_and_weight(mask,&pt,&poly_id,&weight)){
            exit(EXIT_FAILURE);
        }

        if (poly_id >= 0) {
            // rely on short circuiting
            if (weight < 1.0 || drand48() < weight) {
                printf("%.16g %.16g %ld %.16g\n",
                       pt.phi*R2D, 90.-pt.theta*R2D, poly_id, weight); 
                nrand--;
            }
        }
    }
}

int main(int argc, char** argv)
{
    long nrand=0;
    const char* filename=NULL;
    struct MangleMask* mask=NULL;

    if (argc < 3) {
        printf("usage: ./testrand mask_file nrand\n");
        printf("    Generate random points in the mask, according \n"
               "    to the weight of each polygon\n");
        exit(EXIT_FAILURE);
    }

    filename=argv[1];
    nrand=atol(argv[2]);

    mask=mangle_new();

    mangle_read(mask, filename);
    // just prints some metadata
    mangle_print(stderr,mask,1);

    do_genrand(mask, nrand);

    // if using in a library, call this
    //mangle_free(mask);
}
