#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "dims.h"
#include "HCube.h"



struct HCube* HCubeAlloc() {
    struct HCube* h = malloc(sizeof(struct HCube));
    assert(h != NULL);
    return h;
}

struct HCube* HCubeAllocWithBounds(double low[NDIM], double high[NDIM]) {

    struct HCube* h = HCubeAlloc();

    memcpy(h->low, low, NDIM*sizeof(double));
    memcpy(h->high, high, NDIM*sizeof(double));

    return h;

}

struct HCube* HCubeArrayAlloc(size_t n) {

    struct HCube* harray;
    harray = malloc(n*sizeof(struct HCube));
    assert(harray != NULL);
    return harray;

}


void HCubeFree(struct HCube* h) {
    if (h != NULL) {
        free(h);
    }
}


double HCubeDist(struct HCube* h, double p[NDIM]) {

    assert(h != NULL);

    double sum=0;
    double diff=0;

    for(int i=0; i < NDIM; i++) {
        double pdata = p[i];

        double hlow = h->low[i];
        double hhigh = h->high[i];

        if (pdata < hlow) {
            diff = pdata - hlow;
            sum += diff*diff;
        } else if (pdata > hhigh) {
            diff = pdata - hhigh;
            sum += diff*diff;
        }
    }

    return sqrt(sum);
}

int HCubeContains(struct HCube* h, double p[NDIM]) {

    double dist = HCubeDist(h, p);
    if (dist > 0) {
        return 0;
    } else {
        return 1;
    }

}
