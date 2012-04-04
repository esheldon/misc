#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "healpix.h"
#include "defs.h"
#include "vector.h"

/* need -std=gnu99 since c99 doesn't have getopt */
void process_args(int argc, char** argv, 
                  int64* nside, double* rad_radians, int* rad_in_file) {

    int c;
    double rad_arcsec=-1;
    int help=0;
    while ((c = getopt(argc, argv, "n:r:h")) != -1) {
        switch (c) {
            case 'n':
                *nside = (int64) atoi(optarg);
                break;
            case 'r':
                rad_arcsec = atof(optarg);
                break;
            case 'h':
                help=1;
                break;
            default:
                break;
        }
    }

    if (help) {
        wlog(
        "usage:\n"
        "    cat data | pixelof [options] > result\n\n"
        "  -n nside nside for healpix, power of two, default 4096 which \n"
        "  -r rad   search radius in arcsec. If not sent, must be third \n"
        "           column in file2, in which case it can be different \n"
        "           for each point.\n"
        "  -h print this message and exit\n");
        exit(EXIT_FAILURE);
    }

    if (rad_arcsec <= 0) {
        *rad_in_file=1;
    } else {
        *rad_in_file=0;
        *rad_radians = rad_arcsec/3600.*D2R; 
    }

}

int main(int argc, char** argv) {
    int64 nside=4096;
    double rad_radians=-1;
    int rad_in_file=0;
    process_args(argc, argv, &nside, &rad_radians, &rad_in_file);

    struct healpix* hpix = hpix_new(nside);

    struct vector* pixlist = vector_new(0,sizeof(int64));

    double ra=0, dec=0;
    while (2 == fscanf(stdin,"%lf %lf", &ra, &dec)) {
        if (rad_in_file) {
            if (1 != fscanf(stdin,"%lf", &rad_radians)) {
                fprintf(stderr,"failed to read radius\n");
                exit(EXIT_FAILURE);
            }
            rad_radians *= D2R/3600.;
        }

        hpix_disc_intersect_radec(hpix, ra, dec, rad_radians, pixlist);

        printf("%.16g %.16g %lu", ra, dec, pixlist->size);
        int64 *pix_ptr = vector_front(pixlist);
        int64 *end     = vector_end(pixlist);
        while (pix_ptr != end) {
            printf(" %ld", *pix_ptr);
            pix_ptr++;
        }
        printf("\n");
    }
}
