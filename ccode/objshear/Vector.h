// This header was auto-generated
#ifndef _VECTOR_H
#define _VECTOR_H
#include <stdint.h>

#include <stdlib.h>

#ifndef float64
#define float64 double
#endif

struct szvector {
    size_t size;
    size_t* data;
};

struct szvector* szvector_new(size_t num);

// clears all memory in ->data and sets pointer to NULL
void szvector_delete(struct szvector* vector);

void szvector_resize(struct szvector* vector, size_t newsize);

struct i64vector {
    size_t size;
    int64_t* data;
};

struct i64vector* i64vector_new(size_t num);

// clears all memory in ->data and sets pointer to NULL
void i64vector_delete(struct i64vector* vector);

void i64vector_resize(struct i64vector* vector, size_t newsize);

struct f64vector {
    size_t size;
    float64* data;
};

struct f64vector* f64vector_new(size_t num);

// clears all memory in ->data and sets pointer to NULL
void f64vector_delete(struct f64vector* vector);

void f64vector_resize(struct f64vector* vector, size_t newsize);

#endif  // header guard
