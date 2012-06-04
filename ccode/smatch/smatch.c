/*
 * The optimal nside for speed depends on the density
 * Density below in #/sq arcmin
 *           nside
 *           64 128 256 512 1024 2048 4096 8192
 * dens 0.01  -  -   -   *    -    -    -    -
 *      0.10  -  -   -   *    -    -    -    -
 *      0.20  -  -   -   -    *    -    -    -
 *      0.30  -  -   -   -    *    -    -    -
 *      0.40  -  -   -   -    *    -    -    -
 *      0.50  -  -   -   -    *    -    -    -
 *      1.00  -  -   -   -    -    *    -    -
 *      1.50  -  -   -   -    -    *    -    -
 *      2.00  -  -   -   -    -    *    -    -
 *      2.50  -  -   -   -    -    *    -    -
 *      3.00  -  -   -   -    -    *    -    -
 *      3.50  -  -   -   -    -    -    *    -
 *      5.00  -  -   -   -    -    -    *    -
 *
 * But note memory usage grows with nside, especially
 * when fairly large match radii are used
 *
 * Note this is about 20% faster than the tree version
 * in the worst cases I've tried.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "match.h"
#include "cat.h"



/* need -std=gnu99 since c99 doesn't have getopt */
const char* process_args(
        int argc, char** argv, 
        int64* nside, 
        double* radius_arcsec, 
        int64* maxmatch, 
        int* print_dist,
        int* verbose) {

    int c;

    while ((c = getopt(argc, argv, "n:r:m:dv")) != -1) {
        switch (c) {
            case 'n':
                *nside = (int64) atoi(optarg);
                break;
            case 'r':
                *radius_arcsec = atof(optarg);
                break;
            case 'm':
                *maxmatch = (int64) atoi(optarg);
                break;
            case 'd':
                *print_dist=1;
                break;
            case 'v':
                *verbose=1;
                break;
            default:
                break;
        }
    }

    if (optind == argc) {
        printf(
        "usage:\n"
        "    cat file1 | smatch [options] file2 > result\n\n"
        "use smaller list as file2 and stream the larger\n"
        "each line of output is index1 index2\n\n"
        "Also, if there are dups in one list, send that on stdin\n\n"
        "  -r rad   search radius in arcsec. If not sent, must be third \n"
        "           column in file2, in which case it can be different \n"
        "           for each point.\n"
        "  -n nside nside for healpix, power of two, default 4096 which \n"
        "           may use a lot of memory\n"
        "  -m maxmatch\n"
        "           maximum number of matches.  Default is 1.  \n"
        "           maxmatch=0 means return all matches\n"
        "  -d       print out 1-cos(d) where d is the separation \n"
        "           in the third column\n"
        "  -v       print out info and progress in stderr\n");

        exit(EXIT_FAILURE);
    }
    return argv[optind];
}

void print_matches(size_t index, 
                   struct vector* matches, // vector of struct match
                   int64 maxmatch, 
                   int64 print_dist) {

    double cdistmod=0;
    struct match *match=NULL, *end=NULL;
    if (matches->size > 0) {
        if (maxmatch > 0) {
            if (maxmatch < matches->size) {
                // not keeping all, sort and keep the closest matches
                vector_sort(matches, &match_compare);
                vector_resize(matches,maxmatch);
            }
        }

        match = vector_front(matches);
        end   = vector_end(matches);
        while (match != end) {
            printf("%lu %ld", index, match->index);
            if (print_dist) {
                cdistmod = 1.0-match->cos_dist;
                if (cdistmod < 0.) cdistmod=0.;
                printf(" %.16g", cdistmod);
            }
            printf("\n");
            match++;
        }
    }
}

int main(int argc, char** argv) {

    int64 nside = 4096;
    double radius_arcsec = -1;
    int64 maxmatch=1;
    int print_dist=0;
    int verbose=0;

    const char* file = process_args(argc, argv, &nside, &radius_arcsec, 
                                    &maxmatch, &print_dist, &verbose);

    if (verbose) {
        if (radius_arcsec > 0)
            wlog("radius:    %0.1lf arcsec\n", radius_arcsec);
        wlog("nside:     %ld\n", nside);
        wlog("maxmatch:  %ld\n", maxmatch);
        wlog("file:      %s\n", file);
    }

    struct cat* cat = read_cat(file, nside, radius_arcsec, verbose);

    if (verbose) wlog("processing stream\n");

    struct vector *matches = vector_new(0,sizeof(struct match));
    size_t index=0;
    double ra=0, dec=0;
    while (2 == fscanf(stdin,"%lf %lf", &ra, &dec)) {
        cat_match(cat, ra, dec, matches);
        print_matches(index, matches, maxmatch, print_dist);
        index++;
    }

    if (verbose) wlog("processed %lu from stream.\n", index);
}
