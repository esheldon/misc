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
struct vector* vector_resize(struct vector* self, size_t num);

// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
struct vector* vector_realloc(struct vector* self, size_t num);

// completely clears memory in the underlying data vector. Use vector_resize(0)
// if you don't want to clear the memory

void vector_clear(struct vector* self);

// clears all memory and sets pointer to NULL
// usage: v=vector_delete(v);

struct vector* vector_delete(struct vector* self);

// push the input onto the end of the vector if reallocation is needed, size is
// doubled, unless it is empty in case a single element is allocated

void vector_push(struct vector* self, int64_t val);

// pop the last element.  You get back a pointer to the underlying data rather
// than a value.  The size is decremented but no reallocation is performed.  If
// the vector is empty, null is returned.  Note at a later date the returned
// pointer could be invalid, so be careful
void* vector_pop(struct vector* self);

// sort according to the input comparison function
void vector_sort(struct vector* self, int (*compar)(const void *, const void *));


void* vector_front(struct vector* self);
void* vector_back(struct vector* self);
//void* vector_find(struct vector* self, void* something);


#endif

