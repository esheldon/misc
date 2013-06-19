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
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "healpix.h"
#include "match.h"
#include "tree.h"

struct point {
    double x;
    double y;
    double z;
};

struct cat {
    size_t size;
    struct point* pts;

    struct healpix* hpix;
    struct tree_node* tree;

    int radius_in_file;
    double *cos_radius;
};

struct cat* cat_delete(struct cat* cat) {
    if (cat) {
        free(cat->pts);
        free(cat->cos_radius);
        cat->hpix = hpix_delete(cat->hpix);
        cat->tree = tree_delete(cat->tree);

        free(cat);
    }

    return NULL;
}

void* alloc_or_die(size_t nbytes, const char* description) {
    void* data = calloc(nbytes,sizeof(char));
    if (data == NULL) {
        wlog("could not allocate %s\n", description);
        exit(EXIT_FAILURE);
    }
    return data;
}

FILE* open_file(const char* fname) {
    FILE* fptr=fopen(fname, "r");
    if (fptr==NULL) {
        wlog("Could not open file: %s\n", fname);
        exit(EXIT_FAILURE);
    }
    return fptr;
}


#define _COUNTLINES_BUFFSIZE 64
size_t countlines(FILE* fptr) {

    char buff[_COUNTLINES_BUFFSIZE];
    size_t count = 0;
         
    while(fgets(buff,_COUNTLINES_BUFFSIZE,fptr) != NULL) {
        count++;
    }
    return count;
}

void repeat_char(char c, int n) {
    for (int i=0; i<n; i++) {
        fputc(c,stderr);
    }
}
static inline void incr_bar(size_t this_index, size_t ntot, size_t ntoprint, char c)
{
    if ( this_index % (ntot/ntoprint) != 0 ) 
        return;
    fputc(c,stderr);
}


struct cat* read_cat(const char* fname, int64 nside, double radius_arcsec, int verbose) {

    int barsize=70;

    FILE* fptr=open_file(fname);


    size_t nlines=countlines(fptr);
    if (verbose) wlog("    found %lu lines\n", nlines);
    rewind(fptr);

    struct cat* cat = alloc_or_die(sizeof(struct cat), "catalog struct");

    double radius_radians=0;
    if (radius_arcsec <= 0) {
        cat->radius_in_file=1;
        cat->cos_radius = alloc_or_die(nlines*sizeof(double), "cos_radius array");
    } else {
        cat->radius_in_file=0;
        cat->cos_radius = alloc_or_die(sizeof(double), "cos_radius");

        radius_radians = radius_arcsec/3600.*D2R;
        cat->cos_radius[0] = cos(radius_radians);
    }

    cat->hpix=NULL;
    cat->tree=NULL;

    cat->pts = alloc_or_die(nlines*sizeof(struct point),"points");
    cat->size = nlines;

    if (verbose) wlog("    creating hpix\n");
    cat->hpix = hpix_new(nside);

    if (verbose) {
        wlog("    reading and building tree\n");
        repeat_char('.', barsize); wlog("\n");
    }

    double ra=0, dec=0;
    struct i64stack* listpix = i64stack_new(0);

    // this will produce a more balanced tree across the whole sky
    int64 half_npix=cat->hpix->npix/2;

    size_t count=0;
    struct point* pt = &cat->pts[0];
    for (size_t i=0; i<cat->size; i++) {
        if (2 != fscanf(fptr, "%lf %lf", &ra, &dec)) {
            wlog("expected to read point at line %lu\n", i);
            exit(EXIT_FAILURE);
        }
        if (cat->radius_in_file) {
            if (1 != fscanf(fptr, "%lf", &radius_arcsec)) {
                wlog("expected to read radius at line %lu\n", i);
                exit(EXIT_FAILURE);
            }
            radius_radians = radius_arcsec/3600.*D2R;
            cat->cos_radius[i] = cos(radius_radians);
        }

        hpix_eq2xyz(ra,dec,&pt->x,&pt->y,&pt->z);

        hpix_disc_intersect(cat->hpix, pt->x, pt->y, pt->z, radius_radians, 
                            listpix);

        int64* ptr=listpix->data;
        while (ptr < listpix->data + listpix->size) {
            tree_insert(&cat->tree, (*ptr)-half_npix, count);
            ptr++;
        }

        pt++;
        count++;
        if (verbose) incr_bar(i+1, cat->size, barsize, '=');
    }
    listpix=i64stack_delete(listpix);

    if (verbose) wlog("\n");

    if (count != nlines) {
        wlog("expected %lu lines but read %lu\n", nlines, count);
        exit(EXIT_FAILURE);
    }

    if (verbose)
        wlog("fullest node has %lu members\n", tree_most_members(cat->tree));

    fclose(fptr);
    return cat;
}

void process_radec(struct cat* cat, double ra, double dec, 
                   struct matchstack* matches) {

    double x=0,y=0,z=0;
    double cos_radius=0;
    //int64 nmatch=0;

    int64 hpixid = hpix_eq2pix(cat->hpix, ra, dec) - cat->hpix->npix/2;

    struct tree_node* node = tree_find(cat->tree, hpixid);

    matchstack_resize(matches,0);

    if (!cat->radius_in_file) {
        cos_radius = cat->cos_radius[0];
    }
    if (node != NULL) {

        hpix_eq2xyz(ra,dec,&x,&y,&z);

        for (size_t i=0; i<node->indices->size; i++) {
            // index into other list
            size_t cat_ind = node->indices->data[i];

            if (cat->radius_in_file) {
                cos_radius = cat->cos_radius[cat_ind];
            }

            struct point* pt = &cat->pts[cat_ind];

            double cos_angle = pt->x*x + pt->y*y + pt->z*z;

            if (cos_angle > cos_radius) {
                matchstack_push(matches, cat_ind, cos_angle);
            }
        }

    }

    return;
}

void print_matches(size_t index, struct matchstack* matches, int64 maxmatch, 
                   int64 print_dist) {

    double cdistmod=0;
    struct match* match=NULL;

    if (matches->size > 0) {
        if (maxmatch > 0) {
            if (maxmatch < matches->size) {
                // not keeping all, sort and keep the closest matches
                matchstack_sort(matches);
                matchstack_resize(matches, maxmatch);
            }
        }
        match=&matches->data[0];
        for (size_t i=0; i<matches->size; i++) {
            printf("%ld %ld", index, match->index);
            if (print_dist) {
                cdistmod = 1.0-match->cosdist;
                if (cdistmod < 0.) cdistmod=0.;
                printf(" %.16g", cdistmod);
            }
            printf("\n");
            match++;
        }
    }

}
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
        wlog(
        "usage:\n"
        "    cat file1 | smatch [options] file2 > result\n\n"
        "use smaller list as file2 and stream the larger\n"
        "each line of output is index1 index2\n\n"
        "Also, if there are dups in one list, send that on stdin\n\n"
        "Note currently only works for keeping all matches\n\n"
        "options:\n"
        "  -r rad   search radius in arcsec. If not sent, must be third \n"
        "           column in file2, in which case it can be different \n"
        "           for each point.\n"
        "  -n nside nside for healpix, power of two, default 4096 which \n"
        "           may use a lot of memory\n"
        "  -m maxmatch\n"
        "           maximum number of matches.  Default is 1.  \n"
        "           maxmatch=0 means return all matches\n"
        "  -d       print out the distance in degrees in the third column\n"
        "  -v       print out info and progress in stderr\n");

        exit(EXIT_FAILURE);
    }
    return argv[optind];
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

    struct matchstack* matches = matchstack_new();
    size_t index=0;
    double ra=0, dec=0;
    while (2 == fscanf(stdin,"%lf %lf", &ra, &dec)) {
        process_radec(cat, ra, dec, matches);
        print_matches(index, matches, maxmatch, print_dist);
        index++;
    }

    if (verbose) wlog("processed %lu from stream.\n", index);
}
