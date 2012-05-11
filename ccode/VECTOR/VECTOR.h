/*
  VECTOR - A generic vector container written using the C preprocessor.
 
  This approach is generally more appealing for generic types than a compiled
  library because the code is more type safe. At the very least one gets
  compiler warnings about incompatible pointers.  It is impossible to get that
  with a compiled library.
  
  Examples
  --------
  The container can hold basic data types like int or double, but it is more
  interesting to work with a struct.  (Note to work with structs or pointers,
  you must use a typedef. I think typedefs actually make the code less 'local'
  and thus harder to reason about, but I don't see a better way to do it).

 
    #include "VECTOR.h"
    struct test {
        int id;
        double x;
    };
    typedef struct test MyStruct;
    typedef struct test* MyStructp;

    // This declares the underlying vector structures as a new type
    VECTOR_DECLARE(MyStruct);

    // vector of pointers to structs.  Data not owned.
    VECTOR_DECLARE(MyStructp);

    // Now create an actual variable of this type.
    VECTOR(MyStruct) v=NULL;

    // initialize zero visible size. The capacity is 1 internally.
    // Always init before using the vector
    VECTOR_INIT(v);
 

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

    //
    // Safe iteration
    //

    // The foreach macro.  The name you give for the iterator
    // is local to this foreach block.
    VECTOR_FOREACH(iter, v)
        iter->i = someval;
        iter->x = otherval;
    VECTOR_FOREACH_END

    // if you are using C99, you can use a nicer form
    VECTOR_FOREACH2(iter, v) {
        iter->i = someval;
        iter->x = otherval;
    }

    // the above are equivalent to the following
    MyStruct *iter = VECTOR_ITER(v);
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

    VECTOR_RESIZE(v, 25);
    assert(25 == VECTOR_SIZE(v));
 
    // clear sets the visible size to zero, but the underlying storage is
    // unchanged.

    VECTOR_CLEAR(v);
    assert(0==VECTOR_SIZE(v));

    // reallocate the underlying storage capacity.  If the new capacity is
    // smaller than the visible "size", size is also changed, otherwise size
    // stays the same.  Be careful if you have pointers to the underlying data
    // got from VECTOR_GETPTR()

    VECTOR_REALLOC(v,newsize);
    assert(newsize==VECTOR_SIZE(v));

    // drop actually reallocates the underlying storage to size 1 and sets the
    // visible size to zero

    VECTOR_DROP(v);
    assert(0==VECTOR_SIZE(v));
    assert(1==VECTOR_CAPACITY(v));

    // free the vector and the underlying array.  Sets the vector to NULL

    VECTOR_DELETE(v);
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
    // storing pointers in the vector
    // recall MyStructp is a MyStruct*
    //

    VECTOR(MyStructp) v=NULL;
    VECTOR_INIT(v);

    // note we never own the pointers in the vector! So we must allocate and
    // free them separately

    struct test* tvec = calloc(n, sizeof(struct test));

    for (i=0; i<n; i++) {
        MyStruct *t = &tvec[i];
        t->id = i;
        t->x = 2*i;

        // this copies the pointer, not the data
        VECTOR_PUSH(v, t);
    }

    // iteration over a vector of pointers
    MyStruct **iter = VECTOR_ITER(v);
    MyStruct **end  = VECTOR_END(v);
    for (i=0; iter != end; iter++, i++) {
        assert((*iter)->id == i);
    }

    // this does not free the data pointed to by pointers
    VECTOR_DELETE(v);

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
    //   I got the idea from a small header
    //   https://github.com/wmorgan/whistlepig/blob/master/rarray.h
    //   Which he copyright (c) 2011 William Morgan
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

// Use this to declare new vector types
//     e.g.  VECTOR_DECLARE(long);
#define VECTOR_DECLARE(type)                                                 \
struct ppvector_##type {                                                     \
    size_t size;                                                             \
    size_t capacity;                                                         \
    type* data;                                                              \
};

// this is how to declare vector variables in your code
//     e.g. use VECTOR(long) myvar;
#define VECTOR(type) struct ppvector_##type*

// always run this before using the vector
#define VECTOR_INIT(vec) do {                                                \
    (vec) = ( typeof((vec)) ) calloc(1, sizeof( typeof(*(vec)) ) );          \
    (vec)->size = 0;                                                         \
    (vec)->data = calloc(1,sizeof(  typeof( *(vec)->data)  ));               \
    (vec)->capacity=1;                                                       \
} while(0)

//
// metadata access
//
#define VECTOR_SIZE(vec) (vec)->size
#define VECTOR_CAPACITY(vec) (vec)->capacity

//
// safe iterators, even for empty vectors
//
#define VECTOR_ITER(vec) (vec)->data
#define VECTOR_END(vec) (vec)->data + (vec)->size


// this should always work since new block is created
// Use like this, e.g. for a vector of int
//
// VECTOR_FOREACH(iter, vec)
//     printf("val is: %d\n", *iter);
// VECTOR_FOREACH_END
//
// The name 'iter' will not live past the foreach
#define VECTOR_FOREACH(iter, vec)  do {                                      \
        typeof(vec->data) (iter) = VECTOR_ITER((vec));                       \
        typeof(vec->data) _iter_end_##vec = VECTOR_END((vec));               \
        for (; (iter) != _iter_end_##vec; (iter)++) {                        \


#define VECTOR_FOREACH_END  } } while (0);

// This version requires C99 for the index declaration in
// the for loop.  Use like this for an int vector
//
// VECTOR_FOREACH2(iter, vec) {
//     printf("val is: %d\n", *iter);
// }
#define VECTOR_FOREACH2(iter, vec)                                           \
    for(typeof(vec->data) (iter)=VECTOR_ITER(vec),                           \
        _iter_end=VECTOR_END(vec);                                           \
        iter != _iter_end;                                                   \
        iter++)

//
// safe way to add elements to the vector
//

#define VECTOR_PUSH(vec, val) do {                                           \
    if ((vec)->size == (vec)->capacity) {                                    \
        VECTOR_REALLOC(vec, (vec)->capacity*2);                              \
    }                                                                        \
    (vec)->size++;                                                           \
    (vec)->data[(vec)->size-1] = val;                                        \
} while(0)

// safely pop a value off the vector.  If the vector is empty,
// you just get a zeroed object of the type.  Unfortunately
// this requires two copies.
//
// using some magic: leaving val at the end of this
// block lets it become the value in an expression,
// e.g.
//   x = VECTOR_POP(long, vec);
#define VECTOR_POP(vec) ({                                                   \
    typeof( *(vec)->data) val = {0};                                         \
    if ((vec)->size > 0) {                                                   \
        val=(vec)->data[(vec)->size-- -1];                                   \
    }                                                                        \
    val; \
})


//
// unsafe access, no bounds checking
//

#define VECTOR_GET(vec,index) (vec)->data[index]
#define VECTOR_GETFRONT(vec) (vec)->data[0]
#define VECTOR_GETBACK(vec) (vec)->data[(vec)->size-1]

#define VECTOR_GETPTR(vec,index) &(vec)->data[index]

#define VECTOR_SET(vec,index,val) do {                                       \
    (vec)->data[index] = val;                                                \
} while(0)

// unsafe pop, but fast.  One way to safely use it is something
// like
// while (VECTOR_SIZE(v) > 0)
//     data = VECTOR_POPFAST(v);
//
#define VECTOR_POPFAST(vec) (vec)->data[-1 + (vec)->size--]

//
// Modifying the size or capacity
//

// Completely destroy the data and container
// The container is set to NULL
#define VECTOR_DELETE(vec) do {                                              \
    if ((vec)) {                                                             \
        free((vec)->data);                                                   \
        free((vec));                                                         \
        (vec)=NULL;                                                          \
    }                                                                        \
} while(0)


// Change the visible size
// The capacity is only changed if size is larger
// than the existing capacity
#define VECTOR_RESIZE(vec, newsize)  do {                                    \
    if (newsize > (vec)->capacity) {                                         \
        VECTOR_REALLOC(vec, newsize);                                        \
    }                                                                        \
    (vec)->size=newsize;                                                     \
} while(0)

// Set the visible size to zero
#define VECTOR_CLEAR(vec)  do {                                              \
    (vec)->size=0;                                                           \
} while(0)

// delete the data leaving capacity 1 and set size to 0
#define VECTOR_DROP(vec) do {                                                \
    VECTOR_REALLOC(vec,1);                                                   \
    (vec)->size=0;                                                           \
} while(0)


// reallocate the underlying data
// note we don't allow the capacity to drop below 1
#define VECTOR_REALLOC(vec, nsize) do {                                      \
    size_t newsize=nsize;                                                    \
    size_t sizeof_type = sizeof(  typeof( *((vec)->data) ) );                \
    if (newsize < 1) newsize=1;                                              \
                                                                             \
    if (newsize != (vec)->capacity) {                                        \
        (vec)->data =                                                        \
            realloc((vec)->data, newsize*sizeof_type);                       \
        if (!(vec)->data) {                                                  \
            fprintf(stderr,                                                  \
              "VectorError: failed to reallocate to %lu elements of "        \
              "size %lu\n",                                                  \
              (size_t) newsize, sizeof_type);                                \
            exit(EXIT_FAILURE);                                              \
        }                                                                    \
        if (newsize > (vec)->capacity) {                                     \
            size_t num_new_bytes = (newsize-(vec)->capacity)*sizeof_type;    \
            memset((vec)->data + (vec)->capacity, 0, num_new_bytes);         \
        } else if ((vec)->size > newsize) {                                  \
            (vec)->size = newsize;                                           \
        }                                                                    \
                                                                             \
        (vec)->capacity = newsize;                                           \
    }                                                                        \
} while (0)

//
// sorting
//

// convenience function to sort the data.  The compare_func
// must work on your data type!
#define VECTOR_SORT(vec, compare_func) do {                                  \
    size_t sizeof_type = sizeof(  typeof( *((vec)->data) ) );                \
    qsort((vec)->data, (vec)->size, sizeof_type, compare_func);              \
} while (0)

#endif
