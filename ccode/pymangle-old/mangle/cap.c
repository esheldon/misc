#include <stdio.h>
#include <Python.h>
#include "numpy/arrayobject.h" 
#include "cap.h"
#include "point.h"

struct CapVec* 
CapVec_new(npy_intp n) 
{
    struct CapVec* self=NULL;

    self=calloc(1, sizeof(struct CapVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Cap vector");
        return NULL;
    }
    self->data = calloc(n, sizeof(struct Cap));
    if (self->data == NULL) {
        free(self);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Cap vector");
        return NULL;
    }
    self->size = n;
    return self;
}

struct CapVec* CapVec_free(struct CapVec* self)
{
    if (self != NULL) {
        free(self->data);
        free(self);
        self=NULL;
    }
    return self;
}

int is_in_cap(struct Cap* cap, struct Point* pt)
{
    int incap=0;
    double cdot;

    cdot = 1.0 - cap->x*pt->x - cap->y*pt->y - cap->z*pt->z;
    if (cap->cm < 0.0) {
        incap = cdot > (-cap->cm);
    } else {
        incap = cdot < cap->cm;
    }

    return incap;
}

