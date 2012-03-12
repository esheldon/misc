#ifndef _MANGLE_STACK_H
#define _MANGLE_STACK_H

#include <Python.h>
#include "numpy/arrayobject.h" 

struct IntpStack {
    npy_intp size;
    npy_intp allocated_size;
    npy_intp* data;
};


struct IntpStack* IntpStack_new(void);
void IntpStack_realloc(struct IntpStack* self, npy_intp newsize);
void IntpStack_push(struct IntpStack* self, npy_intp val);
struct IntpStack* IntpStack_free(struct IntpStack* self);

#endif
