/*
  VECTOR - A generic vector container written using the C preprocessor.
 
  This approach is generally more appealing for generic types than a compiled
  library because the code is more type safe. At the very least one gets
  compiler warnings about incompatible pointers etc.  It is impossible to get
  that with a compiled library.
  
  The container can hold basic data types like int or double, but it is more
  interesting to work with a struct.  Note to work with structs or pointers,
  you must use a typedef. I'm sorry about this, as I think typedefs make the
  code less 'local' and thus harder to reason about, but I don't see any other
  way to achieve this.
 
    #include "VECTOR.h"
    struct test {
        int id;
        double x;
    };
    typedef struct test MyStruct;
    typedef struct test* MyStructp;

    // This declares the underlying structures as types
    VECTOR_DECLARE(MyStruct);
    VECTOR_DECLARE(MyStructp);

    // Now create an actual variable of this type.
    VECTOR(MyStruct) v=NULL;

    // initialize zero visible size. The capacity is 1 internally.
    VECTOR_INIT(MyStruct, v);
 

    //
    // Push a value.  Always safe.
    //
    MyStruct t;
    t.id = 3;
    t.x = 3.14;
    VECTOR_PUSH(MyStruct,v,t);
    VECTOR_PUSH(MyStruct,v,t);
    VECTOR_PUSH(MyStruct,v,t);
    assert(3 == VECTOR_SIZE(v)); 

    //
    // Safe iteration
    //
    MyStruct *iter = VECTOR_ITER(v);
    MyStruct *end  = VECTOR_END(v);

    for (; iter != end; iter++) {
        // just don't modify the vector size!
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

    VECTOR_SET(v, 5, tnew);
    MyStruct t = VECTOR_GET(v, 5);

    assert(t.id == tnew.id);
    assert(t.x == tnew.x);
 
    //
    // Modifying the visible size or internal capacity
    //

    // resize.  If new size is smaller, storage is unchanged.  If new size is
    // larger, and also larger than the underlying capacity, reallocation
    // occurs

    VECTOR_RESIZE(MyStruct, v, 25);
    assert(25 == VECTOR_SIZE(v));
 
    // clear sets the visible size to zero, but the underlying storage is
    // unchanged.

    VECTOR_CLEAR(MyStruct,v);

    // reallocate the underlying storage capacity.  If the new capacity is
    // smaller than the visible "size", size is also changed, otherwise size
    // stays the same.  Be careful if you have pointers to the underlying data
    // got from VECTOR_GETPTR()

    VECTOR_REALLOC(MyStruct,v,newsize);

    // drop actually reallocates the underlying storage to size 1 and sets the
    // visible size to zero

    VECTOR_DROP(v);

    // free the vector and the underlying array.  Sets the vector to NULL

    VECTOR_DELETE(MyStruct,v);
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
    VECTOR_SORT(MyStruct, v, &MyStruct_compare);


    //
    // storing pointers in the vector
    //

    VECTOR(MyStructp) v=NULL;
    VECTOR_INIT(MyStructp, v);

    // note we never own the pointers in the vector! So we must allocate and
    // free them separately

    struct test* tvec = calloc(n, sizeof(struct test));

    for (i=0; i<n; i++) {
        MyStruct *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer, not the data
        VECTOR_PUSH(MyStructp, v, t);
    }

    // iteration over a vector of pointers
    i=0;
    MyStruct **iter = VECTOR_ITER(v);
    MyStruct **end  = VECTOR_END(v);
    for (; iter != end; iter++) {
        assert((*iter)->id == i);
        i++;
    }

    // this does not free the data
    VECTOR_DELETE(struct_testp, v);

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
    //   I got the general idea from a small header by
    //   William Morgan
    //   https://github.com/wmorgan/whistlepig/blob/master/rarray.h
    //

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

#ifndef _PREPROCESSOR_VECTOR_H_TOKEN
#define _PREPROCESSOR_VECTOR_H_TOKEN

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


// note the use of temporary variable _v is helpful even when not necessary
// because we will get warnings of incompatible types if we pass in objects
// of the wrong type

// Use this to make new vector types
//     e.g.  VECTOR_DECLARE(long);
#define VECTOR_DECLARE(type) \
struct ppvector_##type {\
    size_t size; \
    size_t capacity; \
    type* data; \
};

// this is how to declare vector variables in your code
//     e.g. use VECTOR(long) myvar;
#define VECTOR(type) struct ppvector_##type*

// always run this before using the vector
#define VECTOR_INIT(type, name) do {                              \
    VECTOR(type) _v = calloc(1,sizeof(struct ppvector_##type));   \
    _v->size = 0;                                                 \
    _v->data = calloc(1,sizeof(type));                            \
    _v->capacity=1;                                               \
    (name) = _v;                                                  \
} while(0)

//
// metadata
//
// Safe
#define VECTOR_SIZE(name) (name)->size
#define VECTOR_CAPACITY(name) (name)->capacity

//
// safe iterators, even for empty vectors
//
#define VECTOR_ITER(name) (name)->data
#define VECTOR_END(name) (name)->data + (name)->size

//
// unsafe access, no bounds checking
//

// returns a copy
#define VECTOR_GET(name,index) (name)->data[index]
#define VECTOR_FRONT(name) (name)->data[0]
#define VECTOR_BACK(name) (name)->data[(name)->size-1]
// returns a pointer
#define VECTOR_GETPTR(name,index) &(name)->data[index]

#define VECTOR_SET(name,index,val) do {  \
    (name)->data[index] = val;           \
} while(0)

// safe way to add elements to the vector
#define VECTOR_PUSH(type, name, val) do {                                       \
    VECTOR(type) _v = (name);                                                   \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                \
        _v = (name);                                                            \
    };                                                                          \
    if (_v->size == _v->capacity) {                                             \
        VECTOR_REALLOC(type, name, _v->capacity*2);                             \
    }                                                                           \
    _v->size++;                                                                 \
    _v->data[_v->size-1] = val;                                                 \
} while(0)

// Completely destroy the data and container
// The container is set to NULL
#define VECTOR_DELETE(type, name) do {    \
    VECTOR(type) _v = (name);             \
    if (_v) {                             \
        free(_v->data);                   \
        free(_v);                         \
        (name)=NULL;                      \
    }                                     \
} while(0)


// Change the visible size
// The capacity is only changed if size is larger
// than the existing capacity
#define VECTOR_RESIZE(type, name, newsize)  do {                                \
    VECTOR(type) _v = (name);                                                   \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                \
        _v = (name);                                                            \
    };                                                                          \
    if (newsize > _v->capacity) {                                               \
        VECTOR_REALLOC(type, name, newsize);                                    \
    }                                                                           \
    _v->size=newsize;                                                           \
} while(0)

// Set the visible size to zero
#define VECTOR_CLEAR(type, name)  do {                                          \
    VECTOR(type) _v = (name);                                                   \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                \
        _v = (name);                                                            \
    }                                                                           \
    _v->size=0;                                                                 \
} while(0)

// delete the data leaving capacity 1 and set size to 0
#define VECTOR_DROP(type, name)  do {                                           \
    VECTOR(type) _v = (name);                                                   \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                \
    } else {                                                                    \
        VECTOR_REALLOC(type,name,1);                                            \
        _v->size=0;                                                             \
    }                                                                           \
} while(0)


// reallocate the underlying data
// note we don't allow the capacity to drop below 1
#define VECTOR_REALLOC(type, name, nsize) do {                                  \
    size_t newsize=nsize;                                                       \
    if (newsize < 1) newsize=1;                                                 \
                                                                                \
    VECTOR(type) _v = (name);                                                   \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                \
        _v = (name);                                                            \
    };                                                                          \
    if (newsize != _v->capacity) {                                              \
        _v->data = realloc(_v->data, newsize*sizeof(struct ppvector_##type));   \
        if (!_v->data) {                                                        \
            fprintf(stderr,                                                     \
              "VectorError: failed to reallocate to %lu elements of "           \
              "size %lu\n",                                                     \
              (size_t) newsize, sizeof(type));                                  \
            exit(EXIT_FAILURE);                                                 \
        }                                                                       \
        if (newsize > _v->capacity) {                                           \
            size_t num_new_bytes = (newsize-_v->capacity)*sizeof(type);         \
            type* p = _v->data + _v->capacity;                                  \
            memset(p, 0, num_new_bytes);                                        \
        } else if (_v->size > newsize) {                                        \
            _v->size = newsize;                                                 \
        }                                                                       \
                                                                                \
        _v->capacity = newsize;                                                 \
    }                                                                           \
} while (0)

// convenience function to sort the data.  The compare_func
// must work on your data type!
#define VECTOR_SORT(type, name, compare_func) do {                              \
    VECTOR(type) _v = (name);                                                   \
    if (_v) {                                                                   \
        qsort(_v->data, _v->size, sizeof(type), compare_func);                  \
    }                                                                           \
} while (0)


#endif
