#ifndef _POINT_H
#define _POINT_H

#include <math.h>
#include <gsl/gsl_vector.h>

struct Point {

    // the size of the vector x is also ndim, but
    // we keep a version here for convenience.
    int ndim;
    gsl_vector* x;
    double z;

};

struct Point* PointAlloc(int ndim);
void PointFree(struct Point* p);

// just make sure the pointers are valid
int PointValid(struct Point* p);

int PointComparable(struct Point* p1, struct Point* p2);

int PointCopy(struct Point* dest, struct Point* src);

int PointEqual(struct Point* p1, struct Point* p2);
double PointDist(struct Point* p1, struct Point* p2);


#endif
