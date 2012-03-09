#ifndef _MANGLE_CAP_H
#define _MANGLE_CAP_H

#include <Python.h>
#include "numpy/arrayobject.h" 
#include "point.h"

struct Cap {
    double x;
    double y;
    double z;
    double cm;
};

struct CapVec {
    npy_intp size;
    struct Cap* data;
};


struct CapVec* CapVec_new(npy_intp n);
struct CapVec* CapVec_free(struct CapVec* self);
int is_in_cap(struct Cap* cap, struct Point* pt);
#endif
