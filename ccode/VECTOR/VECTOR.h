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
