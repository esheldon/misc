#include <Python.h>
#include "numpy/arrayobject.h" 
#include <stdlib.h>
#include "polygon.h"
#include "cap.h"
#include "point.h"

struct PolygonVec* PolygonVec_new(npy_intp n) 
{
    struct PolygonVec* self=NULL;

    self=calloc(1, sizeof(struct PolygonVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate Polygon vector");
        return NULL;
    }
    // pointers will be NULL (0)
    self->data = calloc(n, sizeof(struct Polygon));
    if (self->data == NULL) {
        free(self);
        PyErr_Format(PyExc_MemoryError, "Could not allocate Polygon vector %ld", n);
        return NULL;
    }

    self->size = n;
    return self;
}

struct PolygonVec* PolygonVec_free(struct PolygonVec* self)
{
    struct Polygon* ply=NULL;
    npy_intp i=0;
    if (self != NULL) {
        if (self->data!= NULL) {

            ply=self->data;
            for (i=0; i<self->size; i++) {
                ply->cap_vec = CapVec_free(ply->cap_vec);
                ply++;
            }
            free(self->data);

        }
        free(self);
        self=NULL;
    }
    return self;
}

int is_in_poly(struct Polygon* ply, struct Point* pt)
{
    npy_intp i=0;
    struct Cap* cap=NULL;

    int inpoly=1;


    cap = &ply->cap_vec->data[0];
    for (i=0; i<ply->cap_vec->size; i++) {
        inpoly = inpoly && is_in_cap(cap, pt);
        if (!inpoly) {
            break;
        }
        cap++;
    }
    return inpoly;
}
