#ifndef _MANGLE_POLYGON_H
#define _MANGLE_POLYGON_H

#include <Python.h>
#include "numpy/arrayobject.h" 
#include "cap.h"

struct Polygon {

    npy_intp poly_id;
    npy_intp pixel_id; // optional
    double weight;
    double area; // in str

    struct CapVec* cap_vec;

};

struct PolygonVec {
    npy_intp size;
    struct Polygon* data;
};

struct PolygonVec* PolygonVec_new(npy_intp n);
struct PolygonVec* PolygonVec_free(struct PolygonVec* self);
int is_in_poly(struct Polygon* ply, struct Point* pt);

#endif
