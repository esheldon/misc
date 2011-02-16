#include <stdio.h>
#include <stdlib.h>
#include "swigtest.h"

struct F8Vector* F8VectorAlloc(size_t size) {
    struct F8Vector* v = malloc(sizeof(struct F8Vector));

    v->size = size;
    v->data = malloc(size*sizeof(double));
    if (v->data == NULL) {
       printf("Error allocating %ld for F8Vector\n",size); 
       fflush(stdout);
    }

    return v;
}

struct F8Vector* F8VectorRange(size_t size) {
    struct F8Vector* v = F8VectorAlloc(size);
    for (size_t i=0; i<size; i++) {
        v->data[i] = i;
    }
    return v;
}

void F8VectorPrintSome(struct F8Vector* v, size_t n) {
    if (n > v->size) {
        n=v->size;
    }
    printf("{");
    for (size_t i=0; i<n; i++) {
        printf("%g", v->data[i]);
        if (i < (n-1)) {
            printf(", ");
        }
    }
    printf("}\n");
}
