#include <assert.h>
#include "util.h"
#include "Points.h"
#include "PhotoCatalog.h"

struct PhotoCatalog* PhotoCatalogAlloc(size_t size) {
    struct PhotoCatalog* cat=malloc(sizeof(struct PhotoCatalog));
    assert(cat != NULL);

    cat->size = size;

    cat->id = malloc(size*sizeof(int64_t));
    assert(cat->id != NULL);

    cat->pts = PointsAlloc(size);
    assert(cat->pts != NULL);

    return cat;
}

// allocate the num field
void PhotoCatalogMakeNum(struct PhotoCatalog* cat) {
    if (cat->num == NULL) {
        cat->num = malloc(cat->size*sizeof(int));
        assert(cat->num != NULL);
    }
}
void PhotoCatalogFree(struct PhotoCatalog* cat) {
    if (cat != NULL) {
        if (cat->id != NULL) {
            free(cat->id);
        }
        if (cat->pts != NULL) {
            PointsFree(cat->pts);
        }
        if (cat->num != NULL) {
            free(cat->num);
        }
        free(cat);
    }
}


struct PhotoCatalog* PhotoCatalogRead(const char* filename) {
    wlog("Reading PhotoCatalog, NDIM=%d, from file: '%s'\n", NDIM, filename);

    wlog("    Counting lines\n");
    size_t nlines = count_lines(filename);

    struct PhotoCatalog* cat = PhotoCatalogAlloc(nlines);

    wlog("    Reading %ld lines\n", nlines);
    FILE* fptr=fopen(filename,"r");

    double* pdata = cat->pts->data;

    for (size_t i=0; i<nlines; i++) {
        if (!fscanf(fptr, "%ld", &cat->id[i])) {
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

// read one line from the photo file
int PhotoCatalogReadOne(FILE* fptr, int64_t* id, double point[NDIM]) {

    int stat=0;

    if (feof(fptr)) {
        return 0;
    }
    if (!fscanf(fptr, "%ld", id)) {
        return 0;
    }
    for (int dim=0; dim<NDIM; dim++) {
        if (dim == NDIM-1) {
            stat=fscanf(fptr, "%lf\n", &point[dim]);
        } else {
            stat=fscanf(fptr, "%lf", &point[dim]);
        }
        if (!stat) {
            return 0;
        }
    }
    return 1;
}

void PhotoCatalogWrite(const char* filename, struct PhotoCatalog* cat) {
    FILE* fptr = open_or_exit(filename, "w");
    PhotoCatalogWriteSome(fptr,cat,cat->size);
    fclose(fptr);
}
// Write the specified number of lines to the open file pointer
void PhotoCatalogWriteSome(FILE* fptr, struct PhotoCatalog* cat, size_t n) {

    assert(cat != NULL);

    size_t npts = cat->size;
    if (n <= 0) {
        n = npts;
    }

    double* pdata = cat->pts->data;
    for (size_t i=0; i<n; i++) {
        fprintf(fptr, "%ld ", cat->id[i]);

        for (int dim=0; dim<NDIM; dim++) {
            fprintf(fptr, "%g", pdata[i + npts*dim]);
            if (dim < (NDIM-1)) {
                fprintf(fptr, " ");
            }
        }
        fprintf(fptr,"\n");
    }

}

void PhotoCatalogWriteNum(const char* filename, struct PhotoCatalog* cat) {

    assert(cat != NULL);

    if (cat->num == NULL) {
        wlog("Num was not allocated\n");
        exit(1);
    }

    FILE* fptr = open_or_exit(filename, "w");

    size_t npts = cat->size;

    for (size_t i=0; i<npts; i++) {
        fprintf(fptr, "%ld %d\n", cat->id[i], cat->num[i]);
    }

    fclose(fptr);

}

