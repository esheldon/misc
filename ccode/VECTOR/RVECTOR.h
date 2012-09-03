/*
  RVECTOR - A generic vector container written using the C preprocessor.
 
  Each type is a reference type, meaning the container holds pointers and the
  objects can be destroyed automatically using an explicitly specified
  destructor.  If you just want to hold value types, see VECTOR.h  You can
  also just hold a set of pointers with VECTOR.h if you don't need
  the destructor.

  This macro approach is generally more appealing for generic types than a
  compiled library that uses void*.  The macro approach is actually more type
  safe.  The compiler can do type checking and catch errors; at the very least
  one gets compiler warnings, which are quite useful.  These checks are
  impossible with a void* approach.
  
  Examples
  --------
  The container can hold basic data types like int or double, but it is more
  interesting to work with a struct.  Note to work with structs or pointers,
  you must use a typedef; this is a limitation of C macros. 

 
    #include "VECTOR.h"
    struct test {
        int id;
        double x;
    };
    typedef struct test MyStruct;
    typedef struct test* MyStruct_p;

    // This declares a new type, under the hood 
    // it is a struct pprvector_MyStruct
    RVECTOR_DEF(MyStruct);

    // vector of pointers to structs.  Data not owned.
    RVECTOR_DEF(MyStruct_p);

    // Now create an actual variable.
    VECTOR(MyStruct) v=NULL;

    // Initialize to a vector with zero visible size. The capacity is 1
    // internally.  Always init before using the vector

    v = VECTOR_NEW(MyStruct);
 

    //
    // Push a value.  Always safe.
    //
    MyStruct t;
    t.id = 3;
    t.x = 3.14;
    VECTOR_PUSH(v,t);
    VECTOR_PUSH(v,t);
    VECTOR_PUSH(v,t);
    assert(3 == VECTOR_SIZE(v)); 

    // safely pop a value off the vector.  If the vector is empty,
    // you just get a zeroed object of the type.  Unfortunately
    // this requires two copies.

    x = VECTOR_POP(vec);

    //
    // Safe iteration
    //

    // The foreach macro.  In c99 this is quite nice and compact.  Note the
    // name you give for the iterator is local to this foreach block.

    VECTOR_FOREACH(iter, v) {
        iter->i = someval;
        iter->x = otherval;
    }

    // if not using c99, it is a bit more wordy
    VECTOR_FOREACH_BEG(iter, v)
        iter->i = someval;
        iter->x = otherval;
    VECTOR_FOREACH_END


    // the above are equivalent to the following
    MyStruct *iter = VECTOR_BEGIN(v);
    MyStruct *end  = VECTOR_END(v);
    for (; iter != end; iter++) {
        iter->i = someval;
        iter->x = otherval;
    }

    //
    // Direct access.  Bounds not checked
    //

    // get a copy of data at the specified index
    MyStruct t = VECTOR_GET(v,5);

    // pointer to data at the specified index
    MyStruct *tp = VECTOR_GETPTR(v,5);

    // set value at an index
    MyStruct tnew;
    tnew.id = 57;
    tnew.x = -2.7341;

    // VECTOR_SET and VECTOR_GET are really the same, but
    // read better in context
    VECTOR_SET(v, 5) = tnew;
    MyStruct t = VECTOR_GET(v, 5);

    assert(t.id == tnew.id);
    assert(t.x == tnew.x);
 
    //
    // A faster pop that is unsafe; no bounds checking Only use in situation
    // where you are sure of the current size.
    //

    x = VECTOR_POPFAST(v);


    //
    // Modifying the visible size or internal capacity
    //

    // resize.  If new size is smaller, storage is unchanged.  If new size is
    // larger, and also larger than the underlying capacity, reallocation
    // occurs

    VECTOR_RESIZE(v, 25);
    assert(25 == VECTOR_SIZE(v));
 
    // clear sets the visible size to zero, but the underlying storage is
    // unchanged.  This is different from the std::vector in C++ stdlib
    // VECTOR_DROP corresponds the clear in std::vector

    VECTOR_CLEAR(v);
    assert(0==VECTOR_SIZE(v));

    // reallocate the underlying storage capacity.  If the new capacity is
    // smaller than the visible "size", size is also changed, otherwise size
    // stays the same.  Be careful if you have pointers to the underlying data
    // got from VECTOR_GETPTR()

    VECTOR_REALLOC(v,newsize);
    assert(newsize==VECTOR_SIZE(v));

    // drop reallocates the underlying storage to size 1 and sets the
    // visible size to zero

    VECTOR_DROP(v);
    assert(0==VECTOR_SIZE(v));
    assert(1==VECTOR_CAPACITY(v));

    // free the vector and the underlying array.  Sets the vector to NULL

    VECTOR_DEL(v);
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
    VECTOR_SORT(v, &MyStruct_compare);

    //
    // vectors of vectors
    //
    RVECTOR_DEF(VECTOR(long));
    VECTOR(VECTOR(long)) v = VECTOR_NEW(VECTOR(long));;

    VECTOR_PUSH(v, VECTOR_NEW(long));
    VECTOR_PUSH(v, VECTOR_NEW(long));

    // This is the recommended way to delete elements in a vector of vectors
    // You can't use an iterator in this case
    for (size_t i=0; i<VECTOR_SIZE(v); i++) {
        VECTOR_DEL(VECTOR_GET(v,i));
    }
    VECTOR_DEL(v);

    //
    // storing pointers in the vector
    // recall MyStruct_p is a MyStruct*
    //

    VECTOR(MyStruct_p) v = VECTOR_NEW(MyStruct_p);

    // note we never own the pointers in the vector! So we must allocate and
    // free them separately

    MyStruct* tvec = calloc(n, sizeof(MyStruct));

    for (i=0; i<n; i++) {
        MyStruct *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer, not the data
        VECTOR_PUSH(v, t);
    }

    // iteration over a vector of pointers
    MyStruct **iter = VECTOR_BEGIN(v);
    MyStruct **end  = VECTOR_END(v);
    for (i=0; iter != end; iter++, i++) {
        assert((*iter)->id == i);
    }

    // this does not free the data pointed to by pointers
    VECTOR_DEL(v);

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

#ifndef _PREPROCESSOR_RVECTOR_H_TOKEN
#define _PREPROCESSOR_RVECTOR_H_TOKEN

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "VECTOR.h"

// Use this to define new vector types.
//
//     e.g.  RVECTOR_DEF(long);
//
// Note need the both RVECTOR_DEF and _RVECTOR_DEF in order to
// make vectors of vectors due to the fact that concatenation
// does not expand macros.  That is also why we use a typedef.
#define RVECTOR_DEF(type) _RVECTOR_DEF(type)
#define _RVECTOR_DEF(type)                                                   \
typedef struct pprvector_##type {                                            \
    size_t size;                                                             \
    size_t capacity;                                                         \
    type **data;                                                             \
    void (*dstr)();                                                          \
} pprvector_##type; \
typedef pprvector_##type* p_pprvector_##type

// this is how to declare vector variables in your code
//
//     RVECTOR(long) myvar=RVECTOR_NEW(long,dstr);
//
// It is a reference type, and must be initialized
// with RVECTOR_NEW.  The dstr is a destructor to be
// called when an element of the vector is deleted.
// This can be simply "free" or your own function.
// Send NULL if the vector should not own the data.
//
// Note need the both RVECTOR and _RVECTOR in order to
// allow vectors of vectors because concatenation does not expand macros.
#define RVECTOR(type) _RVECTOR(type)
#define _RVECTOR(type) p_pprvector_##type

// Create a new vector.  Note magic leaving the variable at the end of the
// block
#define RVECTOR_NEW(type,destructor) \
    _RVECTOR_NEW(type,destructor)
#define _RVECTOR_NEW(type,destructor) ({                                     \
    RVECTOR(type) _v =  calloc(1, sizeof(pprvector_##type));                 \
    if (!_v) {                                                               \
        fprintf(stderr,                                                      \
                "VectorError: failed to allocate RVECTOR\n");                \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->data = calloc(1,sizeof(type*));                                      \
    if (!_v->data) {                                                         \
        fprintf(stderr,                                                      \
                "VectorError: failed to initialize RVECTOR data\n");         \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->size = 0;                                                            \
    _v->capacity=1;                                                          \
    _v->dstr=(destructor);                                                   \
    _v;                                                                      \
})


/*
 * We can re-use the VECTOR macros.  See VECTOR.h for
 * details of what these mean.
 */

#define RVECTOR_DEL(vec) VECTOR_DEL(vec)

#define RVECTOR_SIZE(vec) VECTOR_SIZE(vec) 
#define RVECTOR_CAPACITY(vec) VECTOR_CAPACITY(vec)

#define RVECTOR_BEGIN(vec) VECTOR_BEGIN(vec)
#define RVECTOR_END(vec) VECTOR_END(vec)

#define RVECTOR_FOREACH_BEG(itername, vec) VECTOR_FOREACH_BEG(itername, vec)  
#define RVECTOR_FOREACH_END VECTOR_FOREACH_END

#define RVECTOR_FOREACH(itername, vec) VECTOR_FOREACH(itername, vec)

#define RVECTOR_PUSH(vec, val) VECTOR_PUSH(vec, val)

// NOTE ownership transferred
#define RVECTOR_POP(vec) VECTOR_POP(vec)

// unsafe
#define RVECTOR_GET(vec,index) VECTOR_GET(vec,index)
#define RVECTOR_SET(vec,index) VECTOR_SET(vec,index)
#define RVECTOR_GETFRONT(vec) VECTOR_GETFRONT(vec)
#define RVECTOR_GETBACK(vec) VECTOR_GETBACK(vec)
#define RVECTOR_POPFAST(vec) VECTOR_POPFAST(vec)

#define RVECTOR_CLEAR(vec) VECTOR_CLEAR(vec) 
#define RVECTOR_DROP(vec) VECTOR_DROP(vec)
#define RVECTOR_RESERVE(vec, new_capacity) VECTOR_RESERVE(vec, new_capacity)

#define RVECTOR_REALLOC(vec, nsize) VECTOR_REALLOC(vec, nsize)

#define RVECTOR_SORT(vec, compare_func) VECTOR_SORT(vec, compare_func)

#endif
