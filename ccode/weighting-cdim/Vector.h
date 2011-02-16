#ifndef _VECTOR_H
#define _VECTOR_H

#include <stdint.h>

/*
 * 4 byte integers
 */

struct I4Vector {
    size_t size;
    int32_t* data;
};

struct I4Vector* I4VectorAlloc(size_t size);
struct I4Vector* I4VectorZeros(size_t size);
struct I4Vector* I4VectorRealloc(struct I4Vector* v, size_t new_size);
void I4VectorFree(struct I4Vector* v);

/*
 * 8 byte floats
 */
struct F8Vector {
    size_t size;
    double* data;
};

struct F8Vector* F8VectorAlloc(size_t size);
struct F8Vector* F8VectorZeros(size_t size);
struct F8Vector* F8VectorRealloc(struct F8Vector* v, size_t new_size);
void F8VectorFree(struct F8Vector* v);


#endif
