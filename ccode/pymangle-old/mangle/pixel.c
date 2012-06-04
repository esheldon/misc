#include <stdlib.h>
#include <Python.h>
#include "numpy/arrayobject.h" 
#include "mangle.h"
#include "pixel.h"
#include "stack.h"

struct PixelListVec* 
PixelListVec_new(npy_intp n)
{
    struct PixelListVec* self=NULL;
    npy_intp i=0;

    if (n <= 0) {
        PyErr_Format(PyExc_MemoryError, 
                "Vectors must be size > 0, got %ld", n);
        return NULL;
    }
    self=calloc(1, sizeof(struct PixelListVec));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, 
                "Could not allocate pixel list vector");
        return NULL;
    }
    // array of pointers. The pointers will be NULL
    self->data = calloc(n, sizeof(struct IntpStack*));
    if (self->data == NULL) {
        free(self);
        PyErr_Format(PyExc_MemoryError, 
                "Could not allocate %ld pixel list pointers", n);
        return NULL;
    }

    for (i=0; i<n; i++) {
        self->data[i] = IntpStack_new();
    }
    self->size=n;
    return self;
}

struct PixelListVec* 
PixelListVec_free(struct PixelListVec* self)
{
    npy_intp i=0;
    struct IntpStack* s=NULL;
    if (self != NULL) {
        for (i=0; i<self->size; i++) {
            s = self->data[i];
            if (s != NULL) {
                s=IntpStack_free(s);
            }
        }
        free(self);
    }

    return self;
}




int pixel_parse_scheme(char buff[_MANGLE_SMALL_BUFFSIZE], 
                       npy_intp* res, char* pixeltype) {
    int status=1;
    char pixres_buff[_MANGLE_SMALL_BUFFSIZE];
    char* ptr=NULL;
    npy_intp res_bytes=0;

    memset(pixres_buff, 0, _MANGLE_SMALL_BUFFSIZE);

    ptr = strchr(buff, 's');
    if (ptr == NULL) {
        status=0;
        PyErr_Format(PyExc_IOError, "Only support pix scheme s, got: '%s'", buff);
        goto _get_pix_scheme_errout;
    }
    *pixeltype = 's';

    // extract the numerical prefactor, which is the resolution
    res_bytes = (ptr-buff);
    if (res_bytes > 9) {
        status=0;
        PyErr_Format(PyExc_IOError, "pix scheme designation too big: '%s'", buff);
        goto _get_pix_scheme_errout;
    }
    strncpy(pixres_buff, buff, res_bytes);

    if (1 != sscanf(pixres_buff, "%ld", res)) {
        status=0;
        PyErr_Format(PyExc_IOError, 
                "Could not extract resolution from pix scheme: '%s'", buff);
        goto _get_pix_scheme_errout;
    }

_get_pix_scheme_errout:
    return status;
}

npy_intp
get_pixel_simple(npy_intp pixelres, struct Point* pt)
{
    npy_intp pix=0;

    npy_intp i=0;
    npy_intp ps=0, p2=1;
    double cth=0;
    npy_intp n=0, m=0;
    if (pixelres > 0) {
        for (i=0; i<pixelres; i++) { // Work out # pixels/dim and start pix.
            p2  = p2<<1;
            ps += (p2/2)*(p2/2);
        }
      cth = cos(pt->theta);
      n   = (cth==1.0) ? 0: (npy_intp) ( ceil( (1.0-cth)/2 * p2 )-1 );
      m   = (npy_intp) ( floor( (pt->phi/2./M_PI)*p2 ) );
      pix = p2*n+m + ps;

    }
    return pix;
}


