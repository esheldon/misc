#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "defs.h"

struct lensums* lensums_new(size_t nlens, size_t nbin) {
    printf("Creating lensums:\n");
    printf("  nlens: %lu  nbin: %lu\n", nlens, nbin);

    struct lensums* lensums=calloc(1,sizeof(struct lensums));
    if (lensums == NULL) {
        printf("failed to allocate lensums struct\n");
        exit(EXIT_FAILURE);
    }

    lensums->data = calloc(nlens, sizeof(struct lensum));
    if (lensums->data == NULL) {
        printf("failed to allocate lensum array\n");
        exit(EXIT_FAILURE);
    }

    lensums->size = nlens;

    struct lensum* lensum = &lensums->data[0];

    for (size_t i=0; i<nlens; i++) {
        lensum->nbin = nbin;
        lensum->npair = calloc(nbin, sizeof(int64));
        lensum->wsum  = calloc(nbin, sizeof(double));
        lensum->dsum  = calloc(nbin, sizeof(double));
        lensum->osum  = calloc(nbin, sizeof(double));
        lensum->rsum  = calloc(nbin, sizeof(double));

        if (lensum->npair==NULL
                || lensum->wsum==NULL
                || lensum->dsum==NULL
                || lensum->osum==NULL
                || lensum->rsum==NULL) {

            printf("failed to allocate lensum\n");
            exit(EXIT_FAILURE);
        }

        lensum++;
    }
    return lensums;

}

void lensums_print_one(struct lensums* lensums, size_t index) {
    struct lensum* lensum = &lensums->data[index];

    printf("element %ld of lensums:\n",index);
    printf("  zindex: %ld\n", lensum->zindex);
    printf("  weight: %lf\n", lensum->weight);
    printf("  nbin:   %ld\n", lensum->nbin);
    printf("  npair,wsum,dsum,osum,rsum\n");
    for (size_t i=0; i<lensum->nbin; i++) {
        printf("    %ld %lf %lf %lf %lf\n", 
               lensum->npair[i],
               lensum->wsum[i],
               lensum->dsum[i],
               lensum->osum[i],
               lensum->rsum[i]);
    }

}

void lensums_print_firstlast(struct lensums* lensums) {
    lensums_print_one(lensums, 0);
    lensums_print_one(lensums, lensums->size-1);
}

struct lensums* lensums_delete(struct lensums* lensums) {
    if (lensums != NULL) {
        struct lensum* lensum = &lensums->data[0];

        for (size_t i=0; i<lensums->size; i++) {
            free(lensum->npair);
            free(lensum->wsum);
            free(lensum->dsum);
            free(lensum->osum);
            free(lensum->rsum);
            lensum++;
        }
    }
    free(lensums);
    return NULL;
}

struct lensum* lensum_new(size_t nbin) {
    struct lensum* lensum=calloc(1,sizeof(struct lensum));
    if (lensum == NULL) {
        printf("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    lensum->npair = calloc(nbin, sizeof(int64));
    lensum->wsum  = calloc(nbin, sizeof(double));
    lensum->dsum  = calloc(nbin, sizeof(double));
    lensum->osum  = calloc(nbin, sizeof(double));
    lensum->rsum  = calloc(nbin, sizeof(double));

    if (lensum->npair==NULL
            || lensum->wsum==NULL
            || lensum->dsum==NULL
            || lensum->osum==NULL
            || lensum->rsum==NULL) {

        printf("failed to allocate lensum\n");
        exit(EXIT_FAILURE);
    }

    return lensum;
}


void lensum_clear(struct lensum* lensum) {

    lensum->zindex=-1;
    lensum->weight=0;
    for (size_t i=0; i<lensum->nbin; i++) {
        lensum->npair[i] = 0;
        lensum->wsum[i] = 0;
        lensum->dsum[i] = 0;
        lensum->osum[i] = 0;
        lensum->rsum[i] = 0;
    }
}

struct lensum* lensum_delete(struct lensum* lensum) {
    if (lensum != NULL) {
        free(lensum->npair);
        free(lensum->wsum);
        free(lensum->dsum);
        free(lensum->osum);
        free(lensum->rsum);
    }
    free(lensum);
    return NULL;
}
