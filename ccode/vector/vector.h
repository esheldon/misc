/*
  vector - A generic vector container.
 
  Best to give some examples.  See test.c for more examples.
  
  The container can hold basic data types like int or double, but it is more
  interesting to work with a struct
 
    #include "vector.h"
    struct test {
        int id;
        double x;
    };
 
 
    // create an empty vector of test structures
    struct vector* v = vector_new(0,sizeof(struct test));

    // push a value
    struct test t;
    t.id = 3;
    t.x = 3.14;
    vector_push(v,&t);
 
    // push zeroed element on and then fill the value
    struct test* tptr;
    tptr=vector_extend(v);
    tptr->id = 8;
    tptr->x = -88.2;
 
    // pop a value.  Returns a pointer to the last element and decrements the
    // visible size.  Storage is not changed
    tptr = vector_pop(v);


    // safe access, bounds checked
    struct test tnew;
    tnew.id = 57;
    tnew.x = -2.7341;

    vector_set(v, 5, &tnew);
    tptr = vector_get(v, 5);

    assert(tptr->id == tnew.id);
    assert(tptr->x == tnew.x);
 
    // iteration
    struct test* iter  = vector_front(v);
    while (iter != vector_end(v)) {
        iter->i = someval;
        iter++;
    }

    // resize.  If new size is smaller, storage is unchanged.
    // If new size is larger, and also larger than the
    // underlying capacity, reallocation occurs
    vector_resize(v,25);
    assert(v->size == 25);
    assert(vector_size(v) == 25);
 
    // clear sets the visible size to zero, storage remains
    // equivalent to vector_resize(v,0);
    vector_clear(v);

    // reallocate the underlying vector.  If the new capacity
    // is smaller than the visible "size", size is also changed,
    // otherwise size stays the same.  Be careful if you have
    // pointers to the underlying data got from vector_get()
    vector_realloc(v,newsize);

    // freedata actually freeds the underlying storage and sets the
    // size and capacity to zero
    vector_freedata(v);

    // free the vector and the underlying array.  returns NULL.
    v = vector_delete(v);


    // sorting
    // you need a function to sort your structure or type
    // for our struct we can sort by the "id" field
    int compare_test(const void* t1, const void* t2) {
        int temp = 
            ((struct test*) t1)->id 
            -
            ((struct test*) t2)->id ;

        if (temp > 0)
            return 1;
        else if (temp < 0)
            return -1;
        else
            return 0;
    }

    // note only elements [0,size) are sorted
    vector_sort(v, &compare_test);


    // testing
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

#ifndef _GENERIC_VECTOR_H_TOKEN
#define _GENERIC_VECTOR_H_TOKEN
#include <stdint.h>

struct vector {
    size_t size;      // number of elements that are "visible"
    size_t capacity;  // number of allocated elements in data vector

    size_t elsize;    // size of each element
    void* d;
};

// allocated elements are zeroed
struct vector* vector_new(size_t num, size_t elsize);

// if size > allocated size, then a reallocation occurs
// if size <= internal size, then only the ->size field is reset
// use vector_realloc() to reallocate the data vector and set the ->size
void vector_resize(struct vector* self, size_t num);

// Set the size to zero.  The memory for the underlying array is not freed.
void vector_clear(struct vector* self);

// frees the memory allocated for data and sets the size to zero.
// Use vector_clear(v) if you want to set the visible size to zero
// without freeing the underlying array.

void vector_freedata(struct vector* self);


// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
void vector_realloc(struct vector* self, size_t num);

// clears all memory and sets pointer to NULL note if this is a pointer vector,
// the pointers are *not* freed.
//
// usage: v=vector_delete(v);

struct vector* vector_delete(struct vector* self);

//
// push a copy of the data pointed to by the input pointer onto the vector.
//
// if reallocation is needed, size is doubled, unless it is empty in case a
// single element is allocated

void vector_push(struct vector* self, void* val);

// same as push but create an empty, zeroed element at the end rather
// than pushing a particular value on top.  A pointer to the new
// element is returned so you can modify the data.
void* vector_extend(struct vector* self);

size_t _vector_new_push_capacity(struct vector* self);

// pop the last element.  You get back a pointer to the underlying data rather
// than a value.  The size is decremented but no reallocation is performed.  If
// the vector is empty, null is returned.  Note at a later date the returned
// pointer could be invalid, so be careful
void* vector_pop(struct vector* self);

// sort according to the input comparison function
void vector_sort(struct vector* self, 
                 int (*cmp)(const void *, const void *));


// visible size
size_t vector_size(struct vector* self);

// bounds checked access
void* vector_get(struct vector* self, size_t i);

// bounds checked access
void vector_set(struct vector* self, size_t i, void* val);

// get first or last element. If vector is empty, return null.
void* vector_front(struct vector* self);
void* vector_back(struct vector* self);

// don't dereference this!
void* vector_end(struct vector* self);

//void* vector_find(struct vector* self, void* something);


#endif

