#ifndef _TRAIN_CATALOG_H
#define _TRAIN_CATALOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Points.h"

struct TrainCatalog {
    size_t size;
    
    double* zspec;
    double* extra;
    double* weights;

    struct Points* pts;
};

struct TrainCatalog* TrainCatalogAlloc(size_t size);
void TrainCatalogFree(struct TrainCatalog* cat);
struct TrainCatalog* TrainCatalogRead(const char* filename);

void TrainCatalogWrite(const char* filename, struct TrainCatalog* cat);

// Write the specified number of lines to the open file pointer
void TrainCatalogWriteSome(FILE* fptr, struct TrainCatalog* cat, size_t n);

#endif
