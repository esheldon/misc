#ifndef _HCUBE_H
#define _HCUBE_H

#include "Point.h"

struct HCube {
    // The two corners with lowest and highest values
    struct Point* low;
    struct Point* high;

    // this is contained in the low/high but we'll keep
    // a copy here for convenience.
    int ndim;
};

struct HCube* HCubeAlloc(int ndim);

// the low/high points are copied into the allocated HCube
struct HCube* HCubeAllocWithBounds(
        int ndim, 
        struct Point* low, 
        struct Point* high);

void HCubeFree(struct HCube* h);

int HCubeValid(struct HCube* h);
double HCubeDist(struct HCube* h, struct Point* p);
int HCubeContains(struct HCube* h, struct Point* p);



#endif
