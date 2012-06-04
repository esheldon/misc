#ifndef _MANGLE_PIXLIST_H
#define _MANGLE_PIXLIST_H

#include <Python.h>
#include "numpy/arrayobject.h" 
#include "mangle.h"
#include "point.h"
#include "stack.h"

struct PixelListVec {
    char pixeltype;
    npy_intp pixelres;
    npy_intp size;
    struct IntpStack** data;
};


struct PixelListVec* 
PixelListVec_new(npy_intp n);
struct PixelListVec* PixelListVec_free(struct PixelListVec* self);

// extract the pixel scheme and resolution from the input string
// which sould be [res][scheme] e.g. 9s
int pixel_parse_scheme(char buff[_MANGLE_SMALL_BUFFSIZE], 
                       npy_intp* res, char* pixeltype);


npy_intp get_pixel_simple(npy_intp pixelres, struct Point* pt);

#endif

