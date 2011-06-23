#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "../healpix.h"
#include "../Vector.h"
#include "../gcirc.h"
#include "../stack.h"
#include "../defs.h"

#define NRANDOMS 1000

int main(int argc, char** argv) {
    int64 nside=4096;
    struct healpix* hpix = hpix_new(nside);
    printf("nside: %ld\n", nside);
    printf("  npix: %ld\n", hpix->npix);
    printf("  area: %le\n", hpix->area*R2D*R2D);

    // generate random ra/dec between 0 and 1
    double ra[NRANDOMS];
    double dec[NRANDOMS];
    //int64 hpixid[NRANDOMS];
    struct i64vector* hpixid = i64vector_new(NRANDOMS);

    printf("Generating %d random ra,dec in [0,1], [0,1]\n", NRANDOMS);
    srand(time(NULL));
    for (size_t i=0; i< NRANDOMS; i++) {
        ra[i] = ((double)rand())/((double)RAND_MAX);
        dec[i] = ((double)rand())/((double)RAND_MAX);
        hpixid->data[i] = hpix_eq2pix(hpix, ra[i], dec[i]);
    }

    // search rad is 1 degree
    double midra=0.5;
    double middec=0.5;
    double searchrad=0.5*D2R;

    printf("  searchrad: %lf\n", searchrad);
    printf("brute forcing with gcirc\n");

    int count=0;
    double dis,theta;
    for (size_t i=0; i<NRANDOMS; i++) {
        gcirc(midra, middec, ra[i], dec[i], &dis, &theta);
        //printf("ra: %lf dec: %lf  dis: %lf\n", ra[i], dec[i], dis);
        if (dis < searchrad) {
            count++;
        }
    }
    printf("  found %d brute force\n", count);

    printf("Using healpix\n");

    struct i64stack* listpix =i64stack_new(4*hpix->nside);
    hpix_disc_intersect(hpix, midra, middec, searchrad, listpix);
    printf("  %ld pixels intersected\n", listpix->size);

    i64stack_delete(listpix);
    hpix_delete(hpix);

}
