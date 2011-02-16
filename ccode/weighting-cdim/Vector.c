#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "Vector.h"

/*
 * 4 byte integers
 */
struct I4Vector* I4VectorAlloc(size_t size) {
    struct I4Vector* v = malloc(sizeof(struct I4Vector));
    assert(v != NULL);

    v->data = malloc(size*sizeof(int32_t));
    v->size=size;
    return v;
}

struct I4Vector* I4VectorZeros(size_t size) {
    struct I4Vector* v = malloc(sizeof(struct I4Vector));
    assert(v != NULL);

    v->data = calloc(size,sizeof(int32_t));
    v->size=size;
    return v;
}

struct I4Vector* I4VectorRealloc(struct I4Vector* v, size_t new_size) {
    int* newdata = realloc(v->data, new_size*sizeof(int32_t));
    assert(newdata != NULL);
    v->data = newdata;
    v->size = new_size;
    return v;
}
void I4VectorFree(struct I4Vector* v) {
    if (v != NULL) {
        if (v->data != NULL) {
            free(v->data);
        }
        free(v);
    }
}

/*
 * 8 byte floats
 */
struct F8Vector* F8VectorAlloc(size_t size) {
    struct F8Vector* v = malloc(sizeof(struct F8Vector));
    assert(v != NULL);

    v->data = malloc(size*sizeof(double));
    v->size=size;
    return v;
}

struct F8Vector* F8VectorZeros(size_t size) {
    struct F8Vector* v = malloc(sizeof(struct F8Vector));
    assert(v != NULL);

    v->data = calloc(size,sizeof(double));
    v->size=size;
    return v;
}

struct F8Vector* F8VectorRealloc(struct F8Vector* v, size_t new_size) {
    double* newdata = realloc(v->data, new_size*sizeof(double));
    assert(newdata != NULL);
    v->data = newdata;
    v->size = new_size;
    return v;
}
void F8VectorFree(struct F8Vector* v) {
    if (v != NULL) {
        if (v->data != NULL) {
            free(v->data);
        }
        free(v);
    }
}

