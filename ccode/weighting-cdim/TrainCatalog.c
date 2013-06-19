#include <assert.h>
#include "util.h"
#include "Points.h"
#include "TrainCatalog.h"

struct TrainCatalog* TrainCatalogAlloc(size_t size) {
    struct TrainCatalog* cat=malloc(sizeof(struct TrainCatalog));
    assert(cat != NULL);

    cat->size = size;

    cat->zspec = malloc(size*sizeof(double));
    assert(cat->zspec != NULL);

    cat->extra = malloc(size*sizeof(double));
    assert(cat->extra != NULL);

    cat->weights = malloc(size*sizeof(double));
    assert(cat->weights != NULL);

    cat->pts = PointsAlloc(size);
    assert(cat->pts != NULL);

    return cat;
}

void TrainCatalogFree(struct TrainCatalog* cat) {
    if (cat != NULL) {
        if (cat->zspec != NULL) {
            free(cat->zspec);
        }
        if (cat->extra != NULL) {
            free(cat->extra);
        }
        if (cat->weights != NULL) {
            free(cat->weights);
        }
        if (cat->pts != NULL) {
            PointsFree(cat->pts);
        }
        free(cat);
    }
}


struct TrainCatalog* TrainCatalogRead(const char* filename) {
    wlog("Reading TrainCatalog, NDIM=%d, from file: '%s'\n", NDIM, filename);

    wlog("    Counting lines\n");
    size_t nlines = count_lines(filename);

    struct TrainCatalog* cat = TrainCatalogAlloc(nlines);

    wlog("    Reading %ld lines\n", nlines);
    FILE* fptr=fopen(filename,"r");

    double* pdata = cat->pts->data;

    for (size_t i=0; i<nlines; i++) {
        if (!fscanf(fptr, "%lf %lf %lf", 
                    &cat->zspec[i], &cat->extra[i], &cat->weights[i])) {
            perror("Error reading from file: ");
            exit(1);
        }
        // note odd memory layout of the data array
        for (int dim=0; dim<NDIM; dim++) {
            if (!fscanf(fptr, "%lf", &pdata[i + nlines*dim])) {
                perror("Error reading from file: ");
                exit(1);
            }
        }
    }

    fclose(fptr);

    return cat;
}


void TrainCatalogWrite(const char* filename, struct TrainCatalog* cat) {
    FILE* fptr = open_or_exit(filename, "w");
    TrainCatalogWriteSome(fptr,cat,cat->size);
    fclose(fptr);
}
// Write the specified number of lines to the open file pointer
void TrainCatalogWriteSome(FILE* fptr, struct TrainCatalog* cat, size_t n) {

    assert(cat != NULL);

    size_t npts = cat->size;
    if (n <= 0) {
        n = npts;
    }

    double* pdata = cat->pts->data;

    for (size_t i=0; i<n; i++) {
        fprintf(fptr, "%g %g %g ", 
                cat->zspec[i], cat->extra[i], cat->weights[i]);

        for (int dim=0; dim<NDIM; dim++) {
            fprintf(fptr, "%g", pdata[i + npts*dim]);
            if (dim < (NDIM-1)) {
                fprintf(fptr, " ");
            }
        }
        fprintf(fptr,"\n");
    }

}

