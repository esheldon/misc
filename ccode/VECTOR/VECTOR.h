/*
  VECTOR - A generic vector container written using the C preprocessor.
 
  Best to give some examples.  See test.c for more examples.
  
  The container can hold basic data types like int or double, but it is more
  interesting to work with a struct.  Note to work with structs or pointers,
  you must use a typedef (sorry!)
 
    #include "vector.h"
    struct test {
        int id;
        double x;
    };
    typedef struct test mystruct;
    typedef struct test* mystructp;

    // This declares the underlying structures
    VECTOR_DECLARE(mystruct);
    VECTOR_DECLARE(mystructp);

    // declare a vector variable to hold the structs
    VECTOR(mystruct) v=NULL;

    // initialize zero size, capacity 1 inside
    VECTOR_INIT(mystruct, v);
 

    //
    // Push a value.  Always safe. Makes a copy
    //
    mystruct t;
    t.id = 3;
    t.x = 3.14;
    VECTOR_PUSH(mystruct,v,t);
    VECTOR_PUSH(mystruct,v,t);
    assert(2 == VECTOR_SIZE(v)); 

    //
    // safe iteration
    //
    mystruct *iter = VECTOR_ITER(v);
    mystruct *end  = VECTOR_END(v);

    for (; iter != end; iter++) {
        // just don't modify the vector size!
        iter->i = someval;
        iter->x = otherval;
    }

    //
    // Direct access.  Bounds not checked
    //

    // get a copy of data at the specified index
    mystruct t = VECTOR_GET(v,5);

    // pointer to data at the specified index
    mystruct* tp = VECTOR_GETPTR(v,5);

    // set value at an index
    mystruct tnew;
    tnew.id = 57;
    tnew.x = -2.7341;

    VECTOR_SET(v, 5, tnew);
    t = VECTOR_GET(v, 5);

    assert(t.id == tnew.id);
    assert(t.x == tnew.x);
 
    //
    // Modifying the visible size or internal capacity
    //

    // resize.  If new size is smaller, storage is unchanged.
    // If new size is larger, and also larger than the
    // underlying capacity, reallocation occurs

    VECTOR_RESIZE(mystruct, v, 10);
    assert(25 == VECTOR_SIZE(v));
 
    // clear sets the visible size to zero, storage remains
    // equivalent to VECTOR_RESIZE(mystruct,v,0);

    VECTOR_CLEAR(mystruct,v);

    // reallocate the underlying vector.  If the new capacity
    // is smaller than the visible "size", size is also changed,
    // otherwise size stays the same.  Be careful if you have
    // pointers to the underlying data got from VECTOR_GETPTR()

    VECTOR_REALLOC(mystruct,v,newsize);

    // drop actually freeds the underlying storage and sets the
    // size to zero and capacity to one

    VECTOR_DROP(v);

    // free the vector and the underlying array.  Sets the
    // vector to NULL

    VECTOR_DELETE(mystruct,v);


    // sorting
    // you need a function to sort your structure or type
    // for our struct we can sort by the "id" field
    int compare_mystruct(const void* t1, const void* t2) {
        int temp = 
            ((mystruct*) t1)->id 
            -
            ((mystruct*) t2)->id ;

        if (temp > 0)
            return 1;
        else if (temp < 0)
            return -1;
        else
            return 0;
    }

    // note only elements [0,size) are sorted
    VECTOR_SORT(mystruct, v, &compare_mystruct);


    //
    // storing pointers in the vector
    //

    VECTOR(mystructp) v=NULL;
    VECTOR_INIT(mystructp, v);

    // note we never own the pointers in the vector! So we must allocat and
    // free them separately
    struct test* tvec = calloc(n, sizeof(struct test));

    for (i=0; i<n; i++) {
        mystruct *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer to t
        VECTOR_PUSH(mystructp, v, t);
    }

    for (i=0; i<n; i++) {
        mystruct *t = VECTOR_GET(v, i);
        assert(t->id == i);
        assert(t->x == 2*i);
    }

    // iteration
    // could simplify with mystructp types
    i=0;
    mystruct **iter = VECTOR_ITER(v);
    mystruct **end  = VECTOR_END(v);
    for (; iter != end; iter++) {
        assert((*iter)->id == i);
        i++;
    }
    // this does not free the data pointed to!
    VECTOR_DELETE(struct_testp, v);
    // still need to free original vector
    free(tvec);


    //
    // testing
    //

    // to compile the test suite
    python build.py
    ./test

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


// note the use of temporary variable _v is helpful since we will
// get warnings of incompatible types if we screw up

#define VECTOR(type) struct ppvector_##type*

#define VECTOR_DECLARE(type) \
struct ppvector_##type {\
    size_t size; \
    size_t capacity; \
    type* data; \
};

#define VECTOR_SIZE(name) (name)->size
#define VECTOR_CAPACITY(name) (name)->capacity
// returns a copy
#define VECTOR_GET(name,index) (name)->data[index]
#define VECTOR_FRONT(name) (name)->data[0]
#define VECTOR_BACK(name) (name)->data[(name)->size-1]

#define VECTOR_GETPTR(name,index) &(name)->data[index]

#define VECTOR_ITER(name) (name)->data
#define VECTOR_END(name) (name)->data + (name)->size


#define VECTOR_SET(name,index,val) do { \
    (name)->data[index] = val;        \
} while(0)


#define VECTOR_PUSH(type, name, val) do {                                         \
    VECTOR(type) _v = (name);                                                     \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                  \
        _v = (name);                                                            \
    };                                                                          \
    if (_v->size == _v->capacity) {                                             \
        VECTOR_REALLOC(type, name, _v->capacity*2);                               \
    }                                                                           \
    _v->size++;                                                                 \
    _v->data[_v->size-1] = val;                                                 \
} while(0)

#define VECTOR_INIT(type, name) do {                              \
    VECTOR(type) _v = calloc(1,sizeof(struct ppvector_##type));   \
    _v->size = 0;                                               \
    _v->data = calloc(1,sizeof(type));                          \
    _v->capacity=1;                                             \
    (name) = _v;                                                \
} while(0)

#define VECTOR_DELETE(type, name) do {    \
    VECTOR(type) _v = (name);           \
    if (_v) {                         \
        free(_v->data);               \
        free(_v);                     \
        (name)=NULL;                  \
    }                                 \
} while(0)


// capacity only changed if size is larger
#define VECTOR_RESIZE(type, name, newsize)  do {                                  \
    VECTOR(type) _v = (name);                                                     \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                  \
        _v = (name);                                                            \
    };                                                                          \
    if (newsize > _v->capacity) {                                               \
        VECTOR_REALLOC(type, name, newsize);                                      \
    }                                                                           \
    _v->size=newsize;                                                           \
} while(0)

// same as resize to zero
#define VECTOR_CLEAR(type, name)  do {                                            \
    VECTOR(type) _v = (name);                                                     \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                  \
        _v = (name);                                                            \
    }                                                                           \
    _v->size=0;                                                                 \
} while(0)

// delete the data leaving capacity 1 and set size to 0
#define VECTOR_DROP(type, name)  do {                                             \
    VECTOR(type) _v = (name);                                                     \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                  \
    } else {                                                                    \
        VECTOR_REALLOC(type,name,1);                                              \
        _v->size=0;                                                             \
    }                                                                           \
} while(0)



// note we don't allow the capacity to drop below 1
#define VECTOR_REALLOC(type, name, nsize) do {                                    \
    size_t newsize=nsize;                                                       \
    if (newsize < 1) newsize=1;                                                 \
                                                                                \
    VECTOR(type) _v = (name);                                                     \
    if (!_v) {                                                                  \
        VECTOR_INIT(type, name);                                                  \
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


#define VECTOR_SORT(type, name, compare_func) do {                              \
    VECTOR(type) _v = (name);                                                   \
    if (_v) {                                                                   \
        qsort(_v->data, _v->size, sizeof(type), compare_func);                  \
    }                                                                           \
} while (0)


#endif
