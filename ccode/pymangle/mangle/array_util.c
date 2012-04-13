#include <Python.h>
#include "numpy/arrayobject.h" 
#include "array_util.h"

PyObject*
make_intp_array(npy_intp size, const char* name, npy_intp** ptr)
{
    PyObject* array=NULL;
    npy_intp dims[1];
    int ndims=1;
    if (size <= 0) {
        PyErr_Format(PyExc_ValueError, "size of %s array must be > 0",name);
        return NULL;
    }

    dims[0] = size;
    array = PyArray_ZEROS(ndims, dims, NPY_INTP, 0);
    if (array==NULL) {
        PyErr_Format(PyExc_MemoryError, "could not create %s array",name);
        return NULL;
    }

    *ptr = PyArray_DATA((PyArrayObject*)array);
    return array;
}

PyObject*
make_double_array(npy_intp size, const char* name, double** ptr)
{
    PyObject* array=NULL;
    npy_intp dims[1];
    int ndims=1;
    if (size <= 0) {
        PyErr_Format(PyExc_ValueError, "size of %s array must be > 0",name);
        return NULL;
    }

    dims[0] = size;
    array = PyArray_ZEROS(ndims, dims, NPY_FLOAT64, 0);
    if (array==NULL) {
        PyErr_Format(PyExc_MemoryError, "could not create %s array",name);
        return NULL;
    }

    *ptr = PyArray_DATA((PyArrayObject*)array);
    return array;
}
double* 
check_double_array(PyObject* array, const char* name, npy_intp* size)
{
    double* ptr=NULL;
    if (!PyArray_Check(array)) {
        PyErr_Format(PyExc_ValueError,
                "%s must be a numpy array of type 64-bit float",name);
        return NULL;
    }
    if (NPY_DOUBLE != PyArray_TYPE((PyArrayObject*)array)) {
        PyErr_Format(PyExc_ValueError,
                "%s must be a numpy array of type 64-bit float",name);
        return NULL;
    }

    ptr = PyArray_DATA((PyArrayObject*)array);
    *size = PyArray_SIZE((PyArrayObject*)array);

    return ptr;
}

int check_ra_dec_arrays(PyObject* ra_obj, PyObject* dec_obj,
                        double** ra_ptr, npy_intp* nra, 
                        double** dec_ptr, npy_intp*ndec)
{
    if (!(*ra_ptr=check_double_array(ra_obj,"ra",nra)))
        return 0;
    if (!(*dec_ptr=check_double_array(dec_obj,"dec",ndec)))
        return 0;
    if (*nra != *ndec) {
        PyErr_Format(PyExc_ValueError,
                "ra,dec must same length, got (%ld,%ld)",*nra,*ndec);
        return 0;
    }

    return 1;
}
