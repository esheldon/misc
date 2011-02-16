#ifndef _HCUBE_H
#define _HCUBE_H

#include "dims.h"

struct HCube {
    double low[NDIM];
    double high[NDIM];
};

struct HCube* HCubeAlloc();
struct HCube* HCubeAllocWithBounds(
        double low[NDIM], 
        double high[NDIM]);

struct HCube* HCubeArrayAlloc(size_t n);

void HCubeFree(struct HCube* h);
double HCubeDist(struct HCube* h, double p[NDIM]);
int HCubeContains(struct HCube* h, double p[NDIM]);


#endif
