#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "vector.h"

struct vector* vector_new(size_t num, size_t elsize) {
    struct vector* self = calloc(1,sizeof(struct vector));
    if (self == NULL) {
        fprintf(stderr,
          "VectorError: Could not allocate struct vector\n");
        exit(EXIT_FAILURE);
    }

    self->size = num;
    self->capacity = num;
    self->elsize = elsize;

    if (num > 0) {
        self->d = calloc(num, elsize);
        if (self->d == NULL) {
            free(self);
            fprintf(stderr,
          "VectorError: Could not allocate %lu elements of size %lu\n",num,elsize);
            exit(EXIT_FAILURE);
        }
    }

    return self;
}

void vector_realloc(struct vector* self, size_t newsize) {

    size_t oldsize = self->capacity;
    if (newsize != oldsize) {

        void* newdata = realloc(self->d, newsize*self->elsize);
        if (newdata == NULL) {
            fprintf(stderr,
              "VectorError: failed to reallocate to %lu elements of "
              "size %lu\n",
              newsize,self->elsize);
            exit(EXIT_FAILURE);
        }

        if (newsize > oldsize) {
            // the new size is larger.  make sure to initialize the new
            // memory region.  This is the area starting from index [oldsize]
            size_t num_new_bytes = (newsize-oldsize)*self->elsize;
            char* p = newdata + oldsize*self->elsize;
            memset(p, 0, num_new_bytes);
        } else if (self->size > newsize) {
            // The viewed size is larger than the allocated size in this case,
            // we must set the size to the maximum it can be, which is the
            // allocated size
            self->size = newsize;
        }

        self->d = newdata;
        self->capacity = newsize;
    }

}

void vector_resize(struct vector* self, size_t newsize)
{
    if (newsize > self->capacity) {
        vector_realloc(self, newsize);
    }
    self->size = newsize;
}
void vector_clear(struct vector* self)
{
    self->size = 0;
}

void vector_freedata(struct vector* self) {
    if (self->d != NULL) {
        free(self->d);
        self->d=NULL;
    }
    self->size=0;
    self->capacity=0;
}



struct vector* vector_delete(struct vector* self)
{
    if (self) {
        if (self->d) {
            free(self->d);
        }
        free(self);
        self=NULL;
    }
    return self;
}

size_t vector_size(struct vector* self)
{
    return self->size;
}
void* vector_get(struct vector* self, size_t i)
{
    void* p=NULL;
    if (i > self->size) {
        fprintf(stderr,
         "VectorError: Attempt to access element %lu is out "
         " of bounds [0,%lu]\n", i, self->size-1);
        exit(EXIT_FAILURE);
    }
    if (self->size > 0) {
        p = self->d + i*self->elsize;
    }
    return p;
}
void vector_set(struct vector* self, size_t i, void* val)
{
    if (i > self->size) {
        fprintf(stderr,
         "VectorError: Attempt to access element %lu is out "
         " of bounds [0,%lu]\n", i, self->size-1);
        exit(EXIT_FAILURE);
    }

    memcpy(self->d + i*self->elsize,
           val, 
           self->elsize);
}


void* vector_front(struct vector* self)
{
    void* p=NULL;
    if (self->size > 0) {
        p = self->d;
    }
    return p;
}
void* vector_back(struct vector* self)
{
    void* p=NULL;
    if (self->size > 0) {
        p = self->d + (self->size-1)*self->elsize;
    }
    return p;
}
void* vector_end(struct vector* self)
{
    void* p=NULL;
    if (self->size > 0) {
        p = self->d + (self->size)*self->elsize;
    }
    return p;
}

void vector_push(struct vector* self, void* val)
{
    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    if (self->size == self->capacity) {
        size_t new_capacity = _vector_new_push_capacity(self);
        vector_realloc(self, new_capacity);
    }

    memcpy(self->d+self->size*self->elsize, 
           val, 
           self->elsize);
    self->size++;
}
void* vector_extend(struct vector* self)
{
    void* p=NULL;

    // see if we have already filled the available data vector
    // if so, reallocate to larger storage
    if (self->size == self->capacity) {
        size_t new_capacity = _vector_new_push_capacity(self);
        vector_realloc(self, new_capacity);
    }

    p = self->d+self->size*self->elsize;
    memset(p, 0, self->elsize);
    self->size++;

    return p;
}

size_t _vector_new_push_capacity(struct vector* self)
{
    size_t new_capacity;
    if (self->capacity == 0) {
        new_capacity=1;
    } else {
        // this will "floor" the size
        new_capacity = (size_t)(self->capacity*2);
        // we want ceiling
        new_capacity++;
    }
    return new_capacity;
}

void* vector_pop(struct vector* self) {
    void *p=NULL;
    if (self->size > 0) {
        p=self->d + (self->size-1)*self->elsize;
        self->size--;
    }
    return p;
}


void vector_sort(struct vector* self,
                 int (*cmp)(const void *, const void *))
{
    qsort(self->d, self->size, self->elsize, cmp);
}

