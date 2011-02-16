#include <assert.h>
#include "Point.h"
#include "HCube.h"

struct HCube* HCubeAlloc(int ndim) {
    struct HCube* h;
    h = malloc(sizeof(struct HCube));

    assert(h != NULL);

    h->low = PointAlloc(ndim);
    h->high = PointAlloc(ndim);

    h->ndim = ndim;

    return h;
}

struct HCube* HCubeAllocWithBounds(
        int ndim, 
        struct Point* low, 
        struct Point* high) {

    assert(PointValid(low));
    assert(PointValid(high));

    struct HCube* h = HCubeAlloc(ndim);

    PointCopy(h->low, low);
    PointCopy(h->high, high);

    return h;
}



void HCubeFree(struct HCube* h) {
    if (h) {
        if (h->low) {
            PointFree(h->low);
        }
        if (h->high) {
            PointFree(h->high);
        }

        free(h);
    }
}

int HCubeValid(struct HCube* h) {
    if (!h) {
        return 0;
    }
    if (!PointValid(h->low)) {
        return 0;
    }
    if (!PointValid(h->high)) {
        return 0;
    }
    return 1;
}

double HCubeDist(struct HCube* h, struct Point* p) {

    assert(HCubeValid(h));
    assert(PointValid(p));
    assert(h->ndim == p->ndim);

    double sum=0;
    for(int i=0; i < h->ndim; i++) {
        double pdata = p->x->data[i];

        double hlow = h->low->x->data[i];
        double hhigh = h->high->x->data[i];

        double diff=0;
        if (pdata < hlow) {
            diff = hlow-pdata;
            sum += diff*diff;
        }

        if (pdata > hhigh) {
            diff = pdata - hhigh;
            sum += diff;
        }
    }

}

int HCubeContains(struct HCube* h, struct Point* p) {

    double dist = HCubeDist(h, p);
    if (dist == 0) {
        return 1;
    } else {
        return 0;
    }
}
