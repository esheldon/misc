#include <stdlib.h>
#include <stdio.h>
#include "Point.h"
#include "HCube.h"

#include "dims.h"

void test_point() {
    int ndim = 5;
    int i;
    double diff;

    int retval = 0;

    printf("\nTesting points\n");
    struct Point* p1 = PointAlloc(ndim);
    struct Point* p2 = PointAlloc(ndim);
    struct Point* p3;

    for (i=0; i<p1->x->size; i++) {
        p1->x->data[i] = (double) i;
        p2->x->data[i] = (double) 2*i;

        printf("p1->x->data[%d]: %lf\n", i, p1->x->data[i]);
        printf("p2->x->data[%d]: %lf\n", i, p2->x->data[i]);
    }

    if (PointEqual(p1,p2)) {
        printf("Points are equal\n");
    } else {
        printf("Points differ\n");
    }

    diff = PointDist(p1,p2);

    if (diff < 0.0) {
        retval = 1;
    } else {
        printf("diff: %lf\n", diff);
    }

    printf("copying Points\n");
    PointCopy(p2,p1);

    if (PointEqual(p1,p2)) {
        printf("Points are equal\n");
    } else {
        printf("Points differ\n");
    }

    // this would fail
    //PointEqual(p1,p3);

    PointFree(p1);
    PointFree(p2);

}

void test_hcube() {
    printf("\nTesting points\n");

    printf("NDIM = %d\n", NDIM);

    int ndim=2;

    struct Point* low  = PointAlloc(ndim);
    struct Point* high = PointAlloc(ndim);

    low->x->data[0] = 0.0;
    low->x->data[1] = 0.0;

    high->x->data[0] = 1.0;
    high->x->data[1] = 1.0;

    struct HCube* h = HCubeAllocWithBounds(ndim, low, high);

    struct Point* inside = PointAlloc(ndim);
    inside->x->data[0] = 0.5;
    inside->x->data[1] = 0.5;

    if (HCubeContains(h, inside)) {
        printf("inside point correctly found to be contained\n");
    } else {
        printf("inside point incorrectly found to be outside\n");
    }

    struct Point* outside = PointAlloc(ndim);
    outside->x->data[0] = 1.5;
    outside->x->data[1] = 1.5;

    if (HCubeContains(h, outside)) {
        printf("outside point correctly found to be outside\n");
    } else {
        printf("outside point incorrectly found to be inside\n");
    }

    PointFree(low);
    PointFree(high);
    PointFree(inside);
    HCubeFree(h);
}

int main(int argc, char** argv) {
    //test_point();
    test_hcube();
    return 0;
}
