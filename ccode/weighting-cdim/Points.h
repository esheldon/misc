#ifndef _POINT_H
#define _POINT_H

#include "dims.h"

// A set of NDIM dimensional points
struct Points {
    // this is the actual number of points, each of which
    // has NDIM values associated with it
    size_t npts;

    // npts*NDIM
    size_t size;

    // This is the data array
    //
    // this will be NDIM*npts in size.  The layout is such that all data
    // for a given dimension are contiguous in memory.  So the first npts
    // of the array are all from dimension 0, the second npts are from 
    // dimension 1, etc.  This makes for lots of random access when 
    // filling or writing from the array, but speeds up working within
    // a given dimension, such as for partitioning the space.
    //
    // To get the value for point "i" in its dimension "dim", index with
    //     data[i + npts*dim]

    double* data;
};

struct Points* PointsAlloc(size_t npts);
void PointsFree(struct Points* p);


#endif
