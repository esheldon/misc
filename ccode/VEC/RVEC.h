/*
  RVEC - A generic vector container written using the C preprocessor.
 
  This container holds "reference types". The container holds pointers and the
  objects can be destroyed automatically using an explicitly specified
  destructor.  If you just want to hold value types, see VEC.h  You can also
  just hold a set of pointers with VEC.h if you don't need the destructor.

  The macro approach is more appealing for generic types than a compiled
  library that uses void*.  The macro approach is actually more type safe.  The
  compiler can do type checking and catch errors or give warnings.  Such checks
  are impossible with a void* approach.
  
  Examples
  --------
  The container can hold basic data types like int or double, but it is more
  interesting to work with a struct.  Note to work with structs or pointers,
  you must use a typedef; this is a limitation of C macros. 

 
    #include "VEC.h"
    struct test {
        int id;
        double x;
    };
    typedef struct test MyStruct;
    typedef struct test* MyStruct_p;

    // This declares a new type, under the hood 
    // it is a struct pprvector_MyStruct
    RVEC_DEF(MyStruct);

    // vector of pointers to structs.  Data not owned.
    RVEC_DEF(MyStruct_p);

    // Now create an actual variable.
    VEC(MyStruct) v=NULL;

    // Initialize to a vector with zero visible size. The capacity is 1
    // internally.  Always init before using the vector

    v = VEC_NEW(MyStruct);
 

    //
    // Push a value.  Always safe.
    //
    MyStruct t;
    t.id = 3;
    t.x = 3.14;
    VEC_PUSH(v,t);
    VEC_PUSH(v,t);
    VEC_PUSH(v,t);
    assert(3 == VEC_SIZE(v)); 

    // safely pop a value off the vector.  If the vector is empty,
    // you just get a zeroed object of the type.  Unfortunately
    // this requires two copies.

    x = VEC_POP(vec);

    //
    // Safe iteration
    //

    // The foreach macro.  In c99 this is quite nice and compact.  Note the
    // name you give for the iterator is local to this foreach block.

    VEC_FOREACH(iter, v) {
        iter->i = someval;
        iter->x = otherval;
    }

    // if not using c99, it is a bit more wordy
    VEC_FOREACH_BEG(iter, v)
        iter->i = someval;
        iter->x = otherval;
    VEC_FOREACH_END


    // the above are equivalent to the following
    MyStruct *iter = VEC_BEGIN(v);
    MyStruct *end  = VEC_END(v);
    for (; iter != end; iter++) {
        iter->i = someval;
        iter->x = otherval;
    }

    //
    // Direct access.  Bounds not checked
    //

    // get a copy of data at the specified index
    MyStruct t = VEC_GET(v,5);

    // pointer to data at the specified index
    MyStruct *tp = VEC_GETPTR(v,5);

    // set value at an index
    MyStruct tnew;
    tnew.id = 57;
    tnew.x = -2.7341;

    // VEC_SET and VEC_GET are really the same, but
    // read better in context
    VEC_SET(v, 5) = tnew;
    MyStruct t = VEC_GET(v, 5);

    assert(t.id == tnew.id);
    assert(t.x == tnew.x);
 
    //
    // A faster pop that is unsafe; no bounds checking Only use in situation
    // where you are sure of the current size.
    //

    x = VEC_POPFAST(v);


    //
    // Modifying the visible size or internal capacity
    //

    // resize.  If new size is smaller, storage is unchanged.  If new size is
    // larger, and also larger than the underlying capacity, reallocation
    // occurs

    VEC_RESIZE(v, 25);
    assert(25 == VEC_SIZE(v));
 
    // clear sets the visible size to zero, but the underlying storage is
    // unchanged.  This is different from the std::vector in C++ stdlib
    // VEC_DROP corresponds the clear in std::vector

    VEC_CLEAR(v);
    assert(0==VEC_SIZE(v));

    // reallocate the underlying storage capacity.  If the new capacity is
    // smaller than the visible "size", size is also changed, otherwise size
    // stays the same.  Be careful if you have pointers to the underlying data
    // got from VEC_GETPTR()

    VEC_REALLOC(v,newsize);
    assert(newsize==VEC_SIZE(v));

    // drop reallocates the underlying storage to size 1 and sets the
    // visible size to zero

    VEC_DROP(v);
    assert(0==VEC_SIZE(v));
    assert(1==VEC_CAPACITY(v));

    // free the vector and the underlying array.  Sets the vector to NULL

    VEC_FREE(v);
    assert(NULL==v);

    //
    // sorting
    //

    // you need a function to sort your structure or type
    // for our struct we can sort by the "id" field
    int MyStruct_compare(const void* t1, const void* t2) {
        int temp = 
            ((MyStruct*) t1)->id 
            -
            ((MyStruct*) t2)->id ;

        if (temp > 0)
            return 1;
        else if (temp < 0)
            return -1;
        else
            return 0;
    }

    // note only elements [0,size) are sorted
    VEC_SORT(v, &MyStruct_compare);

    //
    // vectors of vectors
    //
    RVEC_DEF(VEC(long));
    VEC(VEC(long)) v = VEC_NEW(VEC(long));;

    VEC_PUSH(v, VEC_NEW(long));
    VEC_PUSH(v, VEC_NEW(long));

    // This is the recommended way to delete elements in a vector of vectors
    // You can't use an iterator in this case
    for (size_t i=0; i<VEC_SIZE(v); i++) {
        VEC_FREE(VEC_GET(v,i));
    }
    VEC_FREE(v);

    //
    // storing pointers in the vector
    // recall MyStruct_p is a MyStruct*
    //

    VEC(MyStruct_p) v = VEC_NEW(MyStruct_p);

    // note we never own the pointers in the vector! So we must allocate and
    // free them separately

    MyStruct* tvec = calloc(n, sizeof(MyStruct));

    for (i=0; i<n; i++) {
        MyStruct *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer, not the data
        VEC_PUSH(v, t);
    }

    // iteration over a vector of pointers
    MyStruct **iter = VEC_BEGIN(v);
    MyStruct **end  = VEC_END(v);
    for (i=0; iter != end; iter++, i++) {
        assert((*iter)->id == i);
    }

    // this does not free the data pointed to by pointers
    VEC_FREE(v);

    // still need to free original vector
    free(tvec);


    //
    // unit tests
    //

    // to compile the test suite
    python build.py
    ./test

    //
    // Acknowledgements
    //   I got the idea from a small header by William Morgan
    //   https://github.com/wmorgan/whistlepig/blob/master/rarray.h


  Copyright (C) 2012  Erin Scott Sheldon, 
                             erin dot sheldon at gmail dot com
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _PREPROCESSOR_RVEC_H_TOKEN
#define _PREPROCESSOR_RVEC_H_TOKEN

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "VEC.h"

// Use this to define new vector types.
//
//     e.g.  RVEC_DEF(long);
//
// Note need the both RVEC_DEF and _RVEC_DEF in order to
// make vectors of vectors due to the fact that concatenation
// does not expand macros.  That is also why we use a typedef.
#define RVEC_DEF(type) _RVEC_DEF(type)
#define _RVEC_DEF(type)                                                   \
typedef struct pprvector_##type {                                            \
    size_t size;                                                             \
    size_t capacity;                                                         \
    int owner;                                                               \
    type **data;                                                             \
    void (*dstr)();                                                          \
} pprvector_##type; \
typedef pprvector_##type* p_pprvector_##type

// this is how to declare vector variables in your code
//
//     RVEC(long) myvar=RVEC_NEW(long,dstr);
//
// It is a reference type, and must be initialized
// with RVEC_NEW.  The dstr is a destructor to be
// called when an element of the vector is deleted.
// This can be simply "free" or your own function.
// Send NULL if the vector should not own the data.
//
// Note need the both RVEC and _RVEC in order to
// allow vectors of vectors because concatenation does not expand macros.
#define RVEC(type) _RVEC(type)
#define _RVEC(type) p_pprvector_##type

// Create a new vector.  Note magic leaving the variable at the end of the
// block
#define RVEC_NEW(type,destructor) \
    _RVEC_NEW(type,destructor)
#define _RVEC_NEW(type,destructor) ({                                     \
    RVEC(type) _v =  calloc(1, sizeof(pprvector_##type));                 \
    if (!_v) {                                                               \
        fprintf(stderr,                                                      \
                "VectorError: failed to allocate RVEC\n");                \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->data = calloc(1,sizeof(type*));                                      \
    if (!_v->data) {                                                         \
        fprintf(stderr,                                                      \
                "VectorError: failed to initialize RVEC data\n");         \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->size = 0;                                                            \
    _v->capacity=1;                                                          \
    _v->dstr=(destructor);                                                   \
    _v;                                                                      \
})


/*
 * We can re-use the VEC macros.  See VEC.h for
 * details of what these mean.
 */

#define RVEC_FREE(vec) VEC_FREE(vec)

#define RVEC_SIZE(vec) VEC_SIZE(vec) 
#define RVEC_CAPACITY(vec) VEC_CAPACITY(vec)

#define RVEC_BEGIN(vec) VEC_BEGIN(vec)
#define RVEC_END(vec) VEC_END(vec)

#define RVEC_FOREACH_BEG(itername, vec) VEC_FOREACH_BEG(itername, vec)  
#define RVEC_FOREACH_END VEC_FOREACH_END

#define RVEC_FOREACH(itername, vec) VEC_FOREACH(itername, vec)

#define RVEC_PUSH(vec, val) VEC_PUSH(vec, val)

// NOTE ownership transferred
#define RVEC_POP(vec) VEC_POP(vec)

// unsafe
#define RVEC_GET(vec,index) VEC_GET(vec,index)
#define RVEC_SET(vec,index) VEC_SET(vec,index)
#define RVEC_GETFRONT(vec) VEC_GETFRONT(vec)
#define RVEC_GETBACK(vec) VEC_GETBACK(vec)
#define RVEC_POPFAST(vec) VEC_POPFAST(vec)

#define RVEC_CLEAR(vec) VEC_CLEAR(vec) 
#define RVEC_DROP(vec) VEC_DROP(vec)
#define RVEC_RESERVE(vec, new_capacity) VEC_RESERVE(vec, new_capacity)

#define RVEC_REALLOC(vec, nsize) VEC_REALLOC(vec, nsize)

#define RVEC_SORT(vec, compare_func) VEC_SORT(vec, compare_func)

#endif
