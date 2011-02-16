#ifndef _PHOTO_CATALOG_H
#define _PHOTO_CATALOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Points.h"

struct PhotoCatalog {
    size_t size;
    
    int64_t* id;

    struct Points* pts;

    // this is to hold the number of times a point was
    // used
    int* num;
};

struct PhotoCatalog* PhotoCatalogAlloc(size_t size);
void PhotoCatalogFree(struct PhotoCatalog* cat);
struct PhotoCatalog* PhotoCatalogRead(const char* filename);
int PhotoCatalogReadOne(FILE* fptr, int64_t* id, double point[NDIM]);

void PhotoCatalogWrite(const char* filename, struct PhotoCatalog* cat);
void PhotoCatalogWriteNum(const char* filename, struct PhotoCatalog* cat);

// Write the specified number of lines to the open file pointer
void PhotoCatalogWriteSome(FILE* fptr, struct PhotoCatalog* cat, size_t n);

// allocate the num field
void PhotoCatalogMakeNum(struct PhotoCatalog* cat);

#endif
