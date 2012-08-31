/*
  VECTOR - A generic vector container written using the C preprocessor.
 
  This approach is generally more appealing for generic types than a compiled
  library that uses void*.  The macro approach is actually more type safe.  The
  compiler can do type checking and catch errors; at the very least one gets
  compiler warnings, which are quite useful.  These checks are impossible
  with a void* approach.

  Note a VECTOR is a container for value types. When pointers are contained,
  the data must be thought of as "not owned".  If you want a container for
  reference types with ownership and constructors/destructores, use a REFVEC
  
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
    // it is a struct ppvector_MyStruct
    VECTOR_DEF(MyStruct);

    // vector of pointers to structs.  Data not owned.
    VECTOR_DEF(MyStruct_p);

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
    VECTOR_DEF(VECTOR(long));
    VECTOR(VECTOR(long)) v = VECTOR_NEW(VECTOR(long));;

    VECTOR_PUSH(v, VECTOR_NEW(long));
    VECTOR_PUSH(v, VECTOR_NEW(long));

    // special method to delete vectors of vectors
    VECTOR_VEC_DEL(v);

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

#ifndef _PREPROCESSOR_VECTOR_H_TOKEN
#define _PREPROCESSOR_VECTOR_H_TOKEN

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Use this to define new vector types.
//
//     e.g.  VECTOR_DEF(long);
//
// Note need the both VECTOR_DEF and _VECTOR_DEF in order to
// make vectors of vectors due to the fact that concatenation
// does not expand macros.  That is also why we use a typedef.
#define VECTOR_DEF(type) _VECTOR_DEF(type)
#define _VECTOR_DEF(type)                                                    \
typedef struct ppvector_##type {                                             \
    size_t size;                                                             \
    size_t capacity;                                                         \
    type* data;                                                              \
    type *(*dstr)();                                                         \
} ppvector_##type; \
typedef ppvector_##type* p_ppvector_##type

// note dstr is not used for regular VECTOR; just in place so we
// can use the same functions like VECTOR_DEL for RVECTOR

// this is how to declare vector variables in your code
//
//     VECTOR(long) myvar=VECTOR_NEW(long);
//
// It is a reference type, and must be initialized
// with VECTOR_NEW.
//
// Note need the both VECTOR and _VECTOR in order to
// allow vectors of vectors because concatenation does not expand macros.
#define VECTOR(type) _VECTOR(type)
#define _VECTOR(type) p_ppvector_##type

// Create a new vector.  Note magic leaving the variable at the end of the
// block
#define VECTOR_NEW(type) _VECTOR_NEW(type)
#define _VECTOR_NEW(type) ({                                                 \
    VECTOR(type) _v =  calloc(1, sizeof(ppvector_##type));                   \
    if (!_v) {                                                               \
        fprintf(stderr,                                                      \
                "VectorError: failed to allocate VECTOR\n");                 \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->data = calloc(1,sizeof(type));                                       \
    if (!_v->data) {                                                         \
        fprintf(stderr,                                                      \
                "VectorError: failed to initialize VECTOR data\n");          \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->size = 0;                                                            \
    _v->capacity=1;                                                          \
    _v;                                                                      \
})


// Completely destroy the data and container
// The container is set to NULL
#define VECTOR_DEL(vec) do {                                                 \
    if ((vec)) {                                                             \
        if ( (vec)->data && (vec)->dstr) {                                   \
            size_t i=0;                                                      \
            for (i=0; i<(vec)->size; i++) {                                  \
                (vec)->dstr( (vec)->data[i] );                               \
            }                                                                \
        }                                                                    \
        free((vec)->data);                                                   \
        free((vec));                                                         \
        (vec)=NULL;                                                          \
    }                                                                        \
} while(0)


// special for vectors of vectors
#define VECTOR_VEC_DEL(vec) do {                                             \
    size_t i=0;                                                              \
    for (i=0; i<VECTOR_SIZE(vec); i++) {                                     \
        VECTOR_DEL(VECTOR_GET(vec,i));                                       \
    }                                                                        \
    VECTOR_DEL(vec);                                                         \
} while(0)

// c99 
//
// We use __v_##vec_##type and assign vec to the same pointer in the for loop
// initializer.  Then vec is a local variable and will go out of scope.  This
// is safer; you can't use vec outside the for block.

#define VECTOR_RAII(type, vec) _VECTOR_RAII(type, vec)
#define _VECTOR_RAII(type, vec)                                              \
    VECTOR(type) __v_##vec_##type = VECTOR_NEW(type);                        \
    for(VECTOR(type) vec =__v_##vec_##type                                   \
        ; vec ; free(vec->data), free(vec), vec=NULL)


//, v->data = calloc(1,sizeof(type))

//
// metadata access
//
#define VECTOR_SIZE(vec) (vec)->size
#define VECTOR_CAPACITY(vec) (vec)->capacity

//
// safe iterators, even for empty vectors
//
#define VECTOR_BEGIN(vec) (vec)->data
#define VECTOR_END(vec) (vec)->data + (vec)->size


// this should always work even with older C since new block is created
//
// usage for an int vector:
//
// VECTOR_FOREACH_BEG(iter, vec)
//     printf("val is: %d\n", *iter);
// VECTOR_FOREACH_END
//
// The name 'iter' will not live past the foreach
#define VECTOR_FOREACH_BEG(itername, vec)  do {                              \
        typeof((vec)->data) (itername) = VECTOR_BEGIN((vec));                \
        typeof((vec)->data) _iter_end_##itername = VECTOR_END((vec));        \
        for (; (itername) != _iter_end_##itername; (itername)++) {           \

#define VECTOR_FOREACH_END  } } while (0);

// This version requires C99 for the index declaration in
// the for loop.
//
// usage for an int vector:
//
// VECTOR_FOREACH(iter, vec) {
//     printf("val is: %d\n", *iter);
// }
#define VECTOR_FOREACH(itername, vec)                                        \
    for(typeof((vec)->data) (itername)=VECTOR_BEGIN(vec),                    \
        _iter_end_##itername=VECTOR_END((vec));                              \
        (itername) != _iter_end_##itername;                                  \
        (itername)++)

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

// safely pop a value off the vector.  If the vector is empty, you just get a
// zeroed object of the type.  Unfortunately this requires two copies.  If the
// vector "owns" the data for a reference type, because you associated a
// destructor, then you should consider the ownership transferred to the
// reciever, so be careful.

// using some magic: leaving val at the end of this
// block lets it become the value in an expression,
// e.g.
//   x = VECTOR_POP(vec);

#define VECTOR_POP(vec) ({                                                   \
    typeof( *(vec)->data) _val = {0};                                        \
    if ((vec)->size > 0) {                                                   \
        _val=(vec)->data[(vec)->size-- -1];                                  \
    }                                                                        \
    _val;                                                                    \
})


//
// unsafe access, no bounds checking
//

#define VECTOR_GET(vec,index) (vec)->data[index]

// VECTOR_SET is the same as VECTOR_GET but reads better
// in a setter context
#define VECTOR_SET(vec,index) (vec)->data[index]

#define VECTOR_GETFRONT(vec) (vec)->data[0]
#define VECTOR_GETBACK(vec) (vec)->data[(vec)->size-1]

#define VECTOR_GETPTR(vec,index) &(vec)->data[index]

// unsafe pop, but fast.  One way to safely use it is something
// like
// while (VECTOR_SIZE(v) > 0)
//     data = VECTOR_POPFAST(v);
//
#define VECTOR_POPFAST(vec) (vec)->data[-1 + (vec)->size--]


//
// Modifying the size or capacity
//

// Change the visible size; note for ref types, the pointers
// will be NULL of the size is increased, so better to PUSH 
// the new elements.
//
// The capacity is only changed if size is larger
// than the existing capacity
#define VECTOR_RESIZE(vec, newsize)  do {                                    \
    if ((newsize) > (vec)->capacity) {                                       \
        VECTOR_REALLOC(vec, newsize);                                        \
    } else if ((newsize) < (vec)->size) {                                    \
        _VECTOR_DSTR_RANGE(vec, (newsize), (vec)->size);                     \
    }                                                                        \
    (vec)->size=(newsize);                                                   \
} while(0)

// execute the destructor on the elements in the range [i1,i2)
#define _VECTOR_DSTR_RANGE(vec, i1, i2) do {                                 \
    if ((vec)->dstr) {                                                       \
        size_t i=0;                                                          \
        for (i=(i1); i< (i2) && i < (vec)->size; i++) {                      \
            fprintf(stderr,"freeing %lu\n", i);                              \
            (vec)->dstr( (vec)->data[i] );                                   \
        }                                                                    \
    }                                                                        \
} while(0)

// Set the visible size to zero and call destructor if needed
#define VECTOR_CLEAR(vec) VECTOR_RESIZE(vec,0)


// reserve at least the specified amount of slots.  If the new capacity is
// smaller than the current capacity, nothing happens.  If larger, a
// reallocation occurs.  No change to current contents will happen.
//
// currently, the exact requested amount is used but in the future we can
// optimize to page boundaries.
#define VECTOR_RESERVE(vec, new_capacity)  do {                              \
    if ((new_capacity) > (vec)->capacity) {                                  \
        VECTOR_REALLOC(vec, new_capacity);                                   \
    }                                                                        \
} while(0)


// delete the data leaving capacity 1 and set size to 0
#define VECTOR_DROP(vec) do {                                                \
    VECTOR_CLEAR(vec);                                                       \
    VECTOR_REALLOC(vec,1);                                                   \
} while(0)


// reallocate the underlying data
// note we don't allow the capacity to drop below 1
#define VECTOR_REALLOC(vec, nsize) do {                                      \
    size_t _newsize=nsize;                                                   \
    size_t _sizeof_type = sizeof(  typeof( *((vec)->data) ) );               \
    if (_newsize < 1) _newsize=1;                                            \
                                                                             \
    if (_newsize != (vec)->capacity) {                                       \
        if (_newsize < (vec)->size) {                                        \
            _VECTOR_DSTR_RANGE(vec, _newsize, (vec)->size);                  \
        }                                                                    \
        (vec)->data =                                                        \
            realloc((vec)->data, _newsize*_sizeof_type);                     \
        if (!(vec)->data) {                                                  \
            fprintf(stderr,                                                  \
              "VectorError: failed to reallocate to %lu elements of "        \
              "size %lu\n",                                                  \
              (size_t) _newsize, _sizeof_type);                              \
            exit(EXIT_FAILURE);                                              \
        }                                                                    \
        if (_newsize > (vec)->capacity) {                                    \
            size_t _num_new_bytes = (_newsize-(vec)->capacity)*_sizeof_type; \
            memset((vec)->data + (vec)->capacity, 0, _num_new_bytes);        \
        } else if ((vec)->size > _newsize) {                                 \
            (vec)->size = _newsize;                                          \
        }                                                                    \
                                                                             \
        (vec)->capacity = _newsize;                                          \
    }                                                                        \
} while (0)

//
// sorting
//

// convenience function to sort the data.  The compare_func
// must work on your data type!
#define VECTOR_SORT(vec, compare_func) do {                                  \
    size_t _sizeof_type = sizeof(  typeof( *((vec)->data) ) );               \
    qsort((vec)->data, (vec)->size, _sizeof_type, compare_func);             \
} while (0)

#endif
