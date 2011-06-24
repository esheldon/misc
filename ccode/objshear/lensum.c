#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "defs.h"


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
