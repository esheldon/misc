%module swigtest
%{
#include "swigtest.h"
%}

%include "swigtest.h"
// we can add a python constructor and
// some methods that wrap our C functions
// we use C++ style for this
%extend F8Vector {// "Attach" these functions to the struct

    F8Vector(size_t size) {
        // use Range just to give us something to play with
        struct F8Vector *v = F8VectorRange(size);

        // NOTE we must return the created object!
        return v;
    }

    ~F8Vector() {
        if ($self->data != NULL) {
            free($self->data);
        }
        free($self);
    }

    // this will become a method!
    void printsome(size_t n) {
        F8VectorPrintSome($self, n);
    }

};
