#ifndef _MANGLE_ARRAY_UTIL_H
#define _MANGLE_ARRAY_UTIL_H
#include <Python.h>
#include "numpy/arrayobject.h" 


PyObject* make_intp_array(npy_intp size, const char* name, npy_intp** ptr);
PyObject* make_double_array(npy_intp size, const char* name, double** ptr);

double* check_double_array(PyObject* array, const char* name, npy_intp* size);
int check_ra_dec_arrays(PyObject* ra_obj, PyObject* dec_obj,
                        double** ra_ptr, npy_intp* nra, 
                        double** dec_ptr, npy_intp*ndec);

#endif
