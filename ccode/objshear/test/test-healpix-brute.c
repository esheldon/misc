#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "../healpix.h"
#include "../Vector.h"
#include "../sort.h"
#include "../histogram.h"
#include "../gcirc.h"
#include "../stack.h"
#include "../defs.h"

#define NRANDOMS 1000

int main(int argc, char** argv) {
    double boxsize=1;
    int showpix=0;

    if (argc < 2) {
        printf("usage: test-healpix-brute nside\n");
        exit(EXIT_FAILURE);
    }
    int64 nside=atoi(argv[1]);
    struct healpix* hpix = hpix_new(nside);

    if (argc > 1) {
        showpix=1;
    }
    printf("nside: %ld\n", nside);
    printf("    npix: %ld\n", hpix->npix);
    printf("    area: %le\n", hpix->area*R2D*R2D);
    printf(" showpix: %d\n", showpix);

    // generate random ra/dec between 0 and 1
    double ra[NRANDOMS];
    double dec[NRANDOMS];
    //int64 hpixid[NRANDOMS];
    struct i64vector* hpixid = i64vector_new(NRANDOMS);

    printf("Generating %d random ra,dec in [0,%lf]\n", NRANDOMS, boxsize);
    srand(time(NULL));
    for (size_t i=0; i< NRANDOMS; i++) {
        ra[i] = boxsize*((double)rand())/((double)RAND_MAX);
        dec[i] = boxsize*((double)rand())/((double)RAND_MAX);
        hpixid->data[i] = hpix_eq2pix(hpix, ra[i], dec[i]);
    }

    // search rad is 1 degree
    double midra=boxsize/2;
    double middec=boxsize/2;
    //double searchrad=boxsize/4*D2R;
    double searchrad=boxsize*0.5*D2R;

    printf("  searchrad: %lf\n", searchrad);
    printf("brute forcing with gcirc\n");

    int bcount=0;
    double dis,theta;
    for (size_t i=0; i<NRANDOMS; i++) {
        gcirc(midra, middec, ra[i], dec[i], &dis, &theta);
        //printf("ra: %lf dec: %lf  dis: %lf\n", ra[i], dec[i], dis);
        if (dis < searchrad) {
            bcount++;
        }
    }
    printf("found %d brute force\n", bcount);




    printf("\nUsing healpix\n");
    printf("  getting sort index\n");
    struct szvector* sind=i64sortind(hpixid);

    printf("  histogramming hpixid\n");
    struct i64vector* h=i64vector_new(0);
    struct szvector* rev=szvector_new(0);
    i64hist1(hpixid, sind, h, rev);

    struct i64stack* listpix =i64stack_new(4*hpix->nside);
    hpix_disc_intersect(hpix, midra, middec, searchrad, listpix);
    printf("  %ld pixels intersected\n", listpix->size);

    int hcount=0;
    int64 minpix=hpixid->data[ sind->data[0] ];
    int64 maxpix=hpixid->data[ sind->data[sind->size-1] ];

    for (size_t i=0; i<listpix->size; i++) {
        int64 pix = listpix->data[i];

        if (showpix) {
            printf("    pixelval: %ld\n", pix);
        }

        if (pix >= minpix && pix <= maxpix) {

            int64 ipix=pix-minpix;
            size_t npix = rev->data[ipix+1] - rev->data[ipix];
            if (npix > 0) {
                for (size_t j=0; j<npix; j++) {
                    size_t ind=rev->data[ rev->data[ipix]+j ];
                    if (ind > (hpixid->size-1)) {
                        printf("ind %ld is out of bounds %ld\n", ind, hpixid->size-1);
                        exit(EXIT_FAILURE);
                    }
                    if (hpixid->data[ind] != pix) {
                        printf("expected pix: %ld got: %ld\n", pix, hpixid->data[ind]);
                        exit(EXIT_FAILURE);
                    }
                    gcirc(midra, middec, ra[ind], dec[ind], &dis, &theta);
                    if (dis < searchrad) {
                        hcount++;
                    }
                }
            }
        }
    }
    printf("found %d healpix\n", hcount);
    if (hcount != bcount) {
        printf("Different counts: brute: %d  hpix: %d\n", bcount, hcount);
        exit(EXIT_FAILURE);
    }

    i64stack_delete(listpix);
    hpix_delete(hpix);
    i64vector_delete(h);
    szvector_delete(rev);
    szvector_delete(sind);

}
