#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "healpix.h"

/* need -std=gnu99 since c99 doesn't have getopt */
int64 process_args(int argc, char** argv) {

    int c;
    int64 nside=4096;
    int help=0;
    while ((c = getopt(argc, argv, "n:h")) != -1) {
        switch (c) {
            case 'n':
                nside = (int64) atoi(optarg);
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
        "  -h print this message and exit\n");
        exit(EXIT_FAILURE);
    }
    return nside;
}

int main(int argc, char** argv) {

    int64 nside = process_args(argc, argv);

    struct healpix* hpix = hpix_new(nside);

    int64 pix=0;
    double ra=0, dec=0;
    while (2 == fscanf(stdin,"%lf %lf", &ra, &dec)) {
        pix = hpix_eq2pix(hpix,ra,dec);
        printf("%.16g %.16g %ld\n", ra, dec, pix);
    }
}
