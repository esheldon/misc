#include <stdio.h>
#include <stdlib.h>
#include "../Vector.h"
#include "../histogram.h"
#include "../defs.h"

int main(int argc, char** argv) {
    // the basic data
    int64 data[] = {1, 5, 0, 3, 3, 8, 1, 9, 0, 9, 1, 3, 5, 9, 8};
    size_t si[] = {2, 8, 0, 6, 10, 3, 4, 11, 1, 12, 5, 14, 7, 9, 13};

    size_t ndata = sizeof(data)/sizeof(int64);

    struct i64vector* vec = i64vector_fromarray(data, ndata);
    struct szvector* sort_index = szvector_fromarray(si, ndata);

    printf("ndata: %ld\n", ndata);
    for (size_t i=0; i< ndata; i++) {
        printf("  vec[%ld]: %ld  si[%ld]: %ld\n", i, data[i], i, si[i]);
    }

    struct i64vector* h = i64vector_new(0);
    struct szvector* rev= szvector_new(0);

    i64hist1(vec, sort_index, h, rev);

    for (size_t i=0; i<h->size; i++) {
        printf("  h[%ld]: %ld\n", i, h->data[i]);
    }
    for (size_t i=0; i<rev->size; i++) {
        printf("  rev[%ld]: %lu\n", i, rev->data[i]);
    }

    i64vector_delete(vec);
    i64vector_delete(h);
    szvector_delete(rev);
    szvector_delete(sort_index);
}
