/*
  VEC - A generic vector container written using the C preprocessor.

  This is a container for value types. When pointers are contained, the data
  must be thought of as "not owned".  If you want a container for reference
  types with ownership and destructore, use a RVEC

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
    // it is a struct ppvector_MyStruct
    VEC_DEF(MyStruct);

    // vector of pointers to structs.  Data not owned.
    VEC_DEF(MyStruct_p);

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
    VEC_DEF(VEC(long));
    VEC(VEC(long)) v = VEC_NEW(VEC(long));;

    VEC_PUSH(v, VEC_NEW(long));
    VEC_PUSH(v, VEC_NEW(long));

    // special method to delete vectors of vectors
    VEC_VEC_FREE(v);

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

#ifndef _PREPROCESSOR_VEC_H_TOKEN
#define _PREPROCESSOR_VEC_H_TOKEN

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Use this to define new vector types.
//
//     e.g.  VEC_DEF(long);
//
// Note need the both VEC_DEF and _VEC_DEF in order to
// make vectors of vectors due to the fact that concatenation
// does not expand macros.  That is also why we use a typedef.
#define VEC_DEF(type) _VEC_DEF(type)
#define _VEC_DEF(type)                                                    \
typedef struct ppvector_##type {                                             \
    size_t size;                                                             \
    size_t capacity;                                                         \
    int owner;                                                               \
    type* data;                                                              \
    void (*dstr)();                                                          \
} ppvector_##type; \
typedef ppvector_##type* p_ppvector_##type

// note dstr is not used for regular VEC; just in place so we
// can use the same functions like VEC_FREE for RVEC

// this is how to declare vector variables in your code
//
//     VEC(long) myvar=VEC_NEW(long);
//
// It is a reference type, and must be initialized
// with VEC_NEW.
//
// Note need the both VEC and _VEC in order to
// allow vectors of vectors because concatenation does not expand macros.
#define VEC(type) _VEC(type)
#define _VEC(type) p_ppvector_##type

// Create a new vector.  Note magic leaving the variable at the end of the
// block
#define VEC_NEW(type) _VEC_NEW(type)
#define _VEC_NEW(type) ({                                                    \
    VEC(type) _v =  calloc(1, sizeof(ppvector_##type));                      \
    if (!_v) {                                                               \
        fprintf(stderr,                                                      \
                "VectorError: failed to allocate VEC\n");                    \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->owner=1;                                                             \
    _v->data = calloc(1,sizeof(type));                                       \
    if (!_v->data) {                                                         \
        fprintf(stderr,                                                      \
                "VectorError: failed to initialize VEC data\n");             \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->size = 0;                                                            \
    _v->capacity=1;                                                          \
    _v;                                                                      \
})

// this is a vector that does not own the data
#define VEC_REFDATA(type, tdata, nelem) ({                                   \
    VEC(type) _v =  calloc(1, sizeof(ppvector_##type));                      \
    if (!_v) {                                                               \
        fprintf(stderr, "VectorError: failed to allocate VEC\n");            \
        exit(EXIT_FAILURE);                                                  \
    }                                                                        \
    _v->owner=0;                                                             \
    _v->data = (tdata);                                                      \
    _v->size = (nelem);                                                      \
    _v->capacity=(nelem);                                                    \
    _v;                                                                      \
})


// assocate with the input data; only works if don't own any data
#define VEC_ASSOC(vec, tdata, nelem) do {                                    \
    if ((vec)->owner) {                                                      \
        fprintf(stderr,"error: attempt to associate data with a "            \
            "vector that owns data\n");                                      \
    } else {                                                                 \
        (vec)->data = (tdata);                                               \
        (vec)->size = (nelem);                                               \
        (vec)->capacity=(nelem);                                             \
    }                                                                        \
} while(0)

// Completely destroy the container and data, if owned
// The container is set to NULL
#define VEC_FREE(vec) do {                                                 \
    if ((vec)) {                                                             \
        if ( (vec)->owner ) {                                                \
            if ( (vec)->data && (vec)->dstr) {                                   \
                size_t i=0;                                                      \
                for (i=0; i<(vec)->size; i++) {                                  \
                    (vec)->dstr( (vec)->data[i] );                               \
                }                                                                \
            }                                                                    \
            free((vec)->data);                                                   \
        }                                                                    \
        free((vec));                                                         \
        (vec)=NULL;                                                          \
    }                                                                        \
} while(0)


// special for vectors of vectors
#define VEC_VEC_FREE(vec) do {                                             \
    size_t i=0;                                                              \
    for (i=0; i<VEC_SIZE(vec); i++) {                                     \
        VEC_FREE(VEC_GET(vec,i));                                       \
    }                                                                        \
    VEC_FREE(vec);                                                         \
} while(0)

// c99 
//
// We use __v_##vec_##type and assign vec to the same pointer in the for loop
// initializer.  Then vec is a local variable and will go out of scope.  This
// is safer; you can't use vec outside the for block.

#define VEC_RAII(type, vec) _VEC_RAII(type, vec)
#define _VEC_RAII(type, vec)                                              \
    VEC(type) __v_##vec_##type = VEC_NEW(type);                        \
    for(VEC(type) vec =__v_##vec_##type                                   \
        ; vec ; free(vec->data), free(vec), vec=NULL)


//, v->data = calloc(1,sizeof(type))

//
// metadata access
//
#define VEC_SIZE(vec) (vec)->size
#define VEC_CAPACITY(vec) (vec)->capacity
#define VEC_OWNER(vec) (vec)->owner

//
// safe iterators, even for empty vectors
//
#define VEC_BEGIN(vec) (vec)->data
#define VEC_END(vec) (vec)->data + (vec)->size


// this should always work even with older C since new block is created
//
// usage for an int vector:
//
// VEC_FOREACH_BEG(iter, vec)
//     printf("val is: %d\n", *iter);
// VEC_FOREACH_END
//
// The name 'iter' will not live past the foreach
#define VEC_FOREACH_BEG(itername, vec)  do {                              \
        typeof((vec)->data) (itername) = VEC_BEGIN((vec));                \
        typeof((vec)->data) _iter_end_##itername = VEC_END((vec));        \
        for (; (itername) != _iter_end_##itername; (itername)++) {           \

#define VEC_FOREACH_END  } } while (0);

// This version requires C99 for the index declaration in
// the for loop.
//
// usage for an int vector:
//
// VEC_FOREACH(iter, vec) {
//     printf("val is: %d\n", *iter);
// }
#define VEC_FOREACH(itername, vec)                                        \
    for(typeof((vec)->data) (itername)=VEC_BEGIN(vec),                    \
        _iter_end_##itername=VEC_END((vec));                              \
        (itername) != _iter_end_##itername;                                  \
        (itername)++)

//
// safe way to add elements to the vector
//

#define VEC_PUSH(vec, val) do {                                           \
    if ((vec)->size == (vec)->capacity) {                                    \
        VEC_REALLOC(vec, (vec)->capacity*2);                              \
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
//   x = VEC_POP(vec);

#define VEC_POP(vec) ({                                                   \
    typeof( *(vec)->data) _val = {0};                                        \
    if ((vec)->size > 0) {                                                   \
        _val=(vec)->data[(vec)->size-- -1];                                  \
    }                                                                        \
    _val;                                                                    \
})


//
// unsafe access, no bounds checking
//

#define VEC_GET(vec,index) (vec)->data[index]

// VEC_SET is the same as VEC_GET but reads better
// in a setter context
#define VEC_SET(vec,index) (vec)->data[index]

#define VEC_GETFRONT(vec) (vec)->data[0]
#define VEC_GETBACK(vec) (vec)->data[(vec)->size-1]

#define VEC_GETPTR(vec,index) &(vec)->data[index]

// unsafe pop, but fast.  One way to safely use it is something
// like
// while (VEC_SIZE(v) > 0)
//     data = VEC_POPFAST(v);
//
#define VEC_POPFAST(vec) (vec)->data[-1 + (vec)->size--]


//
// Modifying the size or capacity
//

// Change the visible size; note for ref types, the pointers
// will be NULL if the size is increased, so better to PUSH 
// the new elements.
//
// The capacity is only changed if size is larger
// than the existing capacity
#define VEC_RESIZE(vec, newsize)  do {                                    \
    if (!(vec)->owner) {                                                     \
        fprintf(stderr,"error: attempt to resize not owned data\n");         \
    } else {                                                                 \
        if ((newsize) > (vec)->capacity) {                                       \
            VEC_REALLOC(vec, newsize);                                        \
        } else if ((newsize) < (vec)->size) {                                    \
            _VEC_DSTR_RANGE(vec, (newsize), (vec)->size);                     \
        }                                                                        \
        (vec)->size=(newsize);                                                   \
    }                                                                            \
} while(0)

// execute the destructor on the elements in the range [i1,i2)
#define _VEC_DSTR_RANGE(vec, i1, i2) do {                                 \
    if ((vec)->dstr) {                                                       \
        size_t i=0;                                                          \
        for (i=(i1); i< (i2) && i < (vec)->size; i++) {                      \
            fprintf(stderr,"freeing %lu\n", i);                              \
            (vec)->dstr( (vec)->data[i] );                                   \
        }                                                                    \
    }                                                                        \
} while(0)

// Set the visible size to zero and call destructor if needed
#define VEC_CLEAR(vec) VEC_RESIZE(vec,0)


// reserve at least the specified amount of slots.  If the new capacity is
// smaller than the current capacity, nothing happens.  If larger, a
// reallocation occurs.  No change to current contents will happen.
//
// currently, the exact requested amount is used but in the future we can
// optimize to page boundaries.
#define VEC_RESERVE(vec, new_capacity)  do {                              \
    if (!(vec)->owner) {                                                     \
        fprintf(stderr,"error: attempt to reserve not owned data\n");        \
    } else {                                                                 \
        if ((new_capacity) > (vec)->capacity) {                                  \
            VEC_REALLOC(vec, new_capacity);                                   \
        }                                                                        \
    }                                                                        \
} while(0)


// delete the data leaving capacity 1 and set size to 0
#define VEC_DROP(vec) do {                                                \
    VEC_CLEAR(vec);                                                       \
    VEC_REALLOC(vec,1);                                                   \
} while(0)


// reallocate the underlying data
// note we don't allow the capacity to drop below 1
#define VEC_REALLOC(vec, nsize) do {                                      \
    if (!(vec)->owner) {                                                     \
        fprintf(stderr,"error: attempt to realloc not owned data\n");        \
    } else {                                                                 \
        size_t _newsize=nsize;                                                   \
        size_t _sizeof_type = sizeof(  typeof( *((vec)->data) ) );               \
        if (_newsize < 1) _newsize=1;                                            \
                                                                                 \
        if (_newsize != (vec)->capacity) {                                       \
            if (_newsize < (vec)->size) {                                        \
                _VEC_DSTR_RANGE(vec, _newsize, (vec)->size);                  \
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
    }                                                                            \
} while (0)

//
// sorting
//

// convenience function to sort the data.  The compare_func
// must work on your data type!
#define VEC_SORT(vec, compare_func) do {                                  \
    size_t _sizeof_type = sizeof(  typeof( *((vec)->data) ) );               \
    qsort((vec)->data, (vec)->size, _sizeof_type, compare_func);             \
} while (0)

#endif
