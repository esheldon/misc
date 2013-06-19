#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "math.h"
#include "source.h"
#include "sort.h"
#include "histogram.h"
#include "Vector.h"

#ifdef SDSSMASK
#include "sdss-survey.h"
#endif

#ifndef WITH_TRUEZ
struct scat* scat_new(size_t n_source, size_t n_zlens) {
    if (n_zlens == 0) {
        printf("scat_new: n_zlens must be > 0\n");
        exit(EXIT_FAILURE);
    }
#else
struct scat* scat_new(size_t n_source) {
#endif

    if (n_source == 0) {
        printf("scat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    //struct scat* scat = malloc(sizeof(struct scat));
    struct scat* scat = calloc(1,sizeof(struct scat));
    if (scat == NULL) {
        printf("Could not allocate struct scat\n");
        exit(EXIT_FAILURE);
    }

    scat->size = n_source;

    //scat->data = malloc(n_source*sizeof(struct source));
    scat->data = calloc(n_source,sizeof(struct source));
    if (scat->data == NULL) {
        printf("Could not allocate %ld sources in scat\n", n_source);
        exit(EXIT_FAILURE);
    }

    // allocate this but keep it zero size for now
    scat->rev=szvector_new(0);

#ifndef WITH_TRUEZ

    scat->zlens = f64vector_new(n_zlens);
    if (scat->zlens== NULL) {
        printf("Could not allocate %ld zlens in scat\n", n_zlens);
        exit(EXIT_FAILURE);
    }

    struct source* src = &scat->data[0];
    for (size_t i=0; i<scat->size; i++) {
        src->scinv = f64vector_new(n_zlens);

        if (src->scinv == NULL) {
            printf("Could not allocate %ld scinv in scat[%ld]\n", n_zlens, i);
            exit(EXIT_FAILURE);
        }

        // for convenience point to this zlens in each source structure
        // DONT FREE!
        src->zlens = scat->zlens;
        src++;
    }
#endif

    return scat;
}

struct scat* scat_read(const char* filename) {
    size_t nread=0;
    printf("Reading sources from %s\n", filename);
    FILE* fptr=fopen(filename,"r");
    if (fptr==NULL) {
        printf("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }


    int64 sigmacrit_style;
    nread=fread(&sigmacrit_style, sizeof(int64), 1, fptr);

#ifndef WITH_TRUEZ
    if (sigmacrit_style != 2) {
        printf("Got sigmacrit_style = %ld but code is compiled for p(z), sigmacrit_style=2.\n", sigmacrit_style);
        exit(EXIT_FAILURE);
    }

    int64 n_zlens;
    nread=fread(&n_zlens, sizeof(int64), 1, fptr);
    printf("    Reading %ld zlens values: ", n_zlens);

    // using a temporary variable since scat is not yet allocated
    struct f64vector* zlens = f64vector_new(n_zlens);
    nread=fread(&zlens->data[0], sizeof(double), n_zlens, fptr);

    printf(" %lf %lf\n", zlens->data[0], zlens->data[n_zlens-1]);

#else
    if (sigmacrit_style != 1) {
        printf("Got sigmacrit_style = %ld but code is compiled for true z, sigmacrit_style=1.\n", sigmacrit_style);
        exit(EXIT_FAILURE);
    }

#endif


    int64 nsource;
    nread=fread(&nsource, sizeof(int64), 1, fptr);
    printf("Reading %ld sources\n", nsource);
    printf("    creating scat...");

#ifndef WITH_TRUEZ
    struct scat* scat=scat_new(nsource, n_zlens);
    // scat->zlens is already allocated
    memcpy(scat->zlens->data, zlens->data, n_zlens*sizeof(double));
    scat->min_zlens = zlens->data[0];
    scat->max_zlens = zlens->data[zlens->size-1];
    f64vector_delete(zlens);
#else
    struct scat* scat=scat_new(nsource);
#endif

    printf("OK\n");

    printf("    reading data...");
    struct source* src = &scat->data[0];
    double ra_rad,dec_rad;
    for (size_t i=0; i<scat->size; i++) {
        nread=fread(&src->ra, sizeof(double), 1, fptr);
        nread=fread(&src->dec, sizeof(double), 1, fptr);
        nread=fread(&src->g1, sizeof(double), 1, fptr);
        nread=fread(&src->g2, sizeof(double), 1, fptr);
        nread=fread(&src->err, sizeof(double), 1, fptr);

        //src->g1 = -src->g1;

#ifndef WITH_TRUEZ

        // read the full inverse critical density for
        // interpolation.  Note these are already allocated
        nread=fread(&src->scinv->data[0], sizeof(double), n_zlens, fptr);

#else
        nread=fread(&src->z, sizeof(double), 1, fptr);

#endif

        ra_rad = src->ra*D2R;
        dec_rad = src->dec*D2R;
        src->sinra = sin(ra_rad);
        src->cosra = cos(ra_rad);
        src->sindec = sin(dec_rad);
        src->cosdec = cos(dec_rad);

#ifdef SDSSMASK
        // add sin(lam),cos(lam),sin(eta),cos(eta)
        eq2sdss_sincos(src->ra,src->dec,
                       &src->sinlam, &src->coslam,
                       &src->sineta, &src->coseta);
#endif

        src++;
    }
    printf("total reads: %lu\n", nread);
    printf("OK\n");

    return scat;
}


void scat_add_hpixid(struct scat* scat, struct healpix* hpix) {

    scat->minpix=INT64_MAX;
    scat->maxpix=-1;
    struct source* src = &scat->data[0];
    for (size_t i=0; i<scat->size; i++) {
        int64 id = hpix_eq2pix(hpix, src->ra, src->dec);
        src->hpixid = id;

        if (id > scat->maxpix) {
            scat->maxpix=id;
        }
        if (id < scat->minpix) {
            scat->minpix=id;
        }
        src++;
    }
}
void scat_add_rev(struct scat* scat, struct healpix* hpix) {
    // need to make a copy of the hpixid in order to get the
    // sorted indices

    struct i64vector* hpixid = i64vector_new(scat->size);
    for (size_t i=0; i<scat->size; i++) {
        hpixid->data[i] = scat->data[i].hpixid;
    }
    // temporary sort index
    struct szvector* sind = i64sortind(hpixid);

    // temporary to hold histogram and sort index
    struct i64vector* h = i64vector_new(0);

    // this is what we'll keep
    struct szvector* rev = szvector_new(0);

    i64hist1(hpixid, sind, h, rev);

    scat->rev = rev;

    // delete temporaries
    hpixid = i64vector_delete(hpixid);
    h      = i64vector_delete(h);
    sind   = szvector_delete(sind);
}



#ifdef WITH_TRUEZ
void scat_add_dc(struct scat* scat, struct cosmo* cosmo) {
    struct source* src = &scat->data[0];
    for (size_t i=0; i<scat->size; i++) {
        src->dc = Dc(cosmo, 0.0, src->z);
        src++;
    }
}
#endif


void scat_print_one(struct scat* scat, size_t el) {
    struct source* src = &scat->data[el];
    printf("element     %ld of scat:\n", el);
    printf("    ra:     %lf  dec: %lf\n", src->ra, src->dec);
    printf("    g1:     %lf  g2: %lf\n", src->g1, src->g2);
    printf("    err:    %lf\n", src->err);
    printf("    hpixid: %ld\n", src->hpixid);

#ifdef SDSSMASK
    printf("    sinlam: %lf\n", src->sinlam);
    printf("    coslam: %lf\n", src->coslam);
    printf("    sineta: %lf\n", src->sineta);
    printf("    coseta: %lf\n", src->coseta);
#endif

#ifdef WITH_TRUEZ
    printf("    z:      %lf\n", src->z);
    printf("    dc:     %lf\n", src->dc);
#else
    size_t nzl = src->zlens->size;
    printf("\n");
    printf("    zlens[0]: %lf  szinv[0]: %lf\n", 
           src->zlens->data[0], src->scinv->data[0]);
    printf("    zlens[%ld]: %lf  szinv[%ld]: %lf\n", 
           nzl-1, src->zlens->data[nzl-1], nzl-1, src->scinv->data[nzl-1]);
#endif
}
void scat_print_firstlast(struct scat* scat) {
    scat_print_one(scat, 0);
    scat_print_one(scat, scat->size-1);
}


// use like this:
//   scat = scat_delete(scat);
// This ensures that the scat pointer is set to NULL
struct scat* scat_delete(struct scat* scat) {

    if (scat != NULL) {

#ifndef WITH_TRUEZ
        scat->zlens = f64vector_delete(scat->zlens);

        struct source* src = &scat->data[0];
        for (size_t i=0; i<scat->size; i++) {
            free(src->scinv);
            src++;
        }
#endif

        free(scat->data);
        scat->rev = szvector_delete(scat->rev);
        free(scat);
    }
    return NULL;
}
