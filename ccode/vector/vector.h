#ifndef _VECTOR_H
#define _VECTOR_H
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

// clears all memory and sets pointer to NULL
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


// bounds checked access
void* vector_get(struct vector* self, size_t i);

// get first or last element. If vector is empty, return null.
void* vector_front(struct vector* self);
void* vector_back(struct vector* self);

// don't dereference this!
void* vector_end(struct vector* self);

//void* vector_find(struct vector* self, void* something);


#endif

