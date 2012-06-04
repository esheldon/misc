#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "math.h"
#include "lens.h"
#include "cosmo.h"

#ifdef SDSSMASK
#include "sdss-survey.h"
#endif

struct lcat* lcat_new(size_t n_lens) {

    if (n_lens == 0) {
        printf("lcat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    //struct lcat* lcat = malloc(sizeof(struct lcat));
    struct lcat* lcat = calloc(1,sizeof(struct lcat));
    if (lcat == NULL) {
        printf("Could not allocate struct lcat\n");
        exit(EXIT_FAILURE);
    }

    lcat->size = n_lens;

    //lcat->data = malloc(n_lens*sizeof(struct lens));
    lcat->data = calloc(n_lens,sizeof(struct lens));
    if (lcat->data == NULL) {
        printf("Could not allocate %ld lenses in lcat\n", n_lens);
        exit(EXIT_FAILURE);
    }

    return lcat;
}

struct lcat* lcat_read(const char* filename) {
    size_t nread=0;
    printf("Reading lenses from %s\n", filename);
    FILE* fptr=fopen(filename,"r");
    if (fptr==NULL) {
        printf("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int64 nlens;

    nread=fread(&nlens, sizeof(int64), 1, fptr);
    printf("Reading %ld lenses\n", nlens);
    printf("    creating lcat...");

    struct lcat* lcat=lcat_new(nlens);
    printf("OK\n");

    printf("    reading data...");
    struct lens* lens = &lcat->data[0];
    double ra_rad,dec_rad;
    for (size_t i=0; i<nlens; i++) {
        nread=fread(&lens->zindex, sizeof(int64), 1, fptr);
        nread=fread(&lens->ra, sizeof(double), 1, fptr);
        nread=fread(&lens->dec, sizeof(double), 1, fptr);
        nread=fread(&lens->z, sizeof(double), 1, fptr);

        ra_rad = lens->ra*D2R;
        dec_rad = lens->dec*D2R;

        lens->sinra = sin(ra_rad);
        lens->cosra = cos(ra_rad);
        lens->sindec = sin(dec_rad);
        lens->cosdec = cos(dec_rad);

        // maskflags must always exist, but it will currently be ignored unless
        // you compile with -DSDSSMASK
        nread=fread(&lens->maskflags, sizeof(int64), 1, fptr);
#ifdef SDSSMASK
        // add sin(lam),cos(lam),sin(eta),cos(eta)
        eq2sdss_sincos(lens->ra,lens->dec,
                       &lens->sinlam, &lens->coslam,
                       &lens->sineta, &lens->coseta);
#endif


        lens++;

    }
    printf("total reads: %lu\n", nread);
    printf("OK\n");

    return lcat;
}

void lcat_add_da(struct lcat* lcat, struct cosmo* cosmo) {
    struct lens* lens = &lcat->data[0];
    for (size_t i=0; i<lcat->size; i++) {
        lens->da = Da(cosmo, 0.0, lens->z);
        lens++;
    }
}

void lcat_print_one(struct lcat* lcat, size_t el) {
    struct lens* lens = &lcat->data[el];
    printf("element %ld of lcat:\n", el);
    printf("    ra:        %lf\n", lens->ra);
    printf("    dec:       %lf\n", lens->dec);
    printf("    z:         %lf\n", lens->z);
    printf("    da:        %lf\n", lens->da);
    printf("    maskflags: %ld\n", lens->maskflags);
#ifdef SDSSMASK
    printf("    sinlam:    %lf\n", lens->sinlam);
    printf("    coslam:    %lf\n", lens->coslam);
    printf("    sineta:    %lf\n", lens->sineta);
    printf("    coseta:    %lf\n", lens->coseta);
#endif
}
void lcat_print_firstlast(struct lcat* lcat) {
    lcat_print_one(lcat, 0);
    lcat_print_one(lcat, lcat->size-1);
}



// use like this:
//   lcat = lcat_delete(lcat);
// This ensures that the lcat pointer is set to NULL
struct lcat* lcat_delete(struct lcat* lcat) {

    if (lcat != NULL) {
        free(lcat->data);
        free(lcat);
    }
    return NULL;
}
