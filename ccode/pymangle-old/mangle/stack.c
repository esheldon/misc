#include <Python.h>
#include "numpy/arrayobject.h" 
#include <stdlib.h>
#include "stack.h"

struct IntpStack* 
IntpStack_new(void) 
{
    struct IntpStack* self=NULL;
    npy_intp start_size=1;

    self=calloc(1, sizeof(struct IntpStack));
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate npy_intp stack");
        return NULL;
    }
    self->data = calloc(start_size, sizeof(npy_intp));
    if (self->data == NULL) {
        free(self);
        PyErr_SetString(PyExc_MemoryError, "Could not allocate npy_intp stack");
        return NULL;
    }
    self->size = 0;
    self->allocated_size=start_size;
    return self;
}

void
IntpStack_realloc(struct IntpStack* self, npy_intp newsize)
{
    npy_intp oldsize=0;
    npy_intp* newdata=NULL;
    npy_intp elsize=0;
    npy_intp num_new_bytes=0;

    oldsize = self->allocated_size;
    if (newsize > oldsize) {
        elsize = sizeof(npy_intp);

        newdata = realloc(self->data, newsize*elsize);
        if (newdata == NULL) {
            printf("failed to reallocate\n");
            exit(EXIT_FAILURE);
        }

        // the allocated size is larger.  make sure to initialize the new
        // memory region.  This is the area starting from index [oldsize]
        num_new_bytes = (newsize-oldsize)*elsize;
        memset(&newdata[oldsize], 0, num_new_bytes);

        self->data = newdata;
        self->allocated_size = newsize;
    }


}

void IntpStack_push(struct IntpStack* self, npy_intp val) {
    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    if (self->size == self->allocated_size) {
        IntpStack_realloc(self, self->size*2);
    }

    self->size++;
    self->data[self->size-1] = val;
}

struct IntpStack* 
IntpStack_free(struct IntpStack* self) 
{
    if (self != NULL) {
        free(self->data);
        free(self);
    }
    return self;
}
