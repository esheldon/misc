#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "math.h"
#include "lens.h"

struct lcat* lcat_new(size_t n_lens) {

    if (n_lens == 0) {
        printf("lcat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    struct lcat* lcat = malloc(sizeof(struct lcat));
    if (lcat == NULL) {
        printf("Could not allocate struct lcat\n");
        exit(EXIT_FAILURE);
    }

    lcat->size = n_lens;

    lcat->data = malloc(n_lens*sizeof(struct lens));
    if (lcat->data == NULL) {
        printf("Could not allocate %ld lenses in lcat\n", n_lens);
        exit(EXIT_FAILURE);
    }

    return lcat;
}


// use like this:
//   lcat = lcat_delete(lcat);
// This ensures that the lcat pointer is set to NULL
struct lcat* lcat_delete(struct lcat* lcat) {

    if (lcat != NULL) {
        free(lcat->data);
        free(lcat);
    }
    return NULL;
}
