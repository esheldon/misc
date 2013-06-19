#ifndef _OBJECT_HGUARD
#define _OBJECT_HGUARD

#define OBJECT_NFIELDS 11

#include "shape.h"

struct object {
    char model[20];
    double row;
    double col;
    struct shape shape;
    double T;
    double counts;

    char psf_model[20];
    struct shape psf_shape;
    double psf_T;
};

int object_read_one(struct object *self, FILE *fobj);
void object_write_one(struct object *self, FILE* fobj);

#endif
