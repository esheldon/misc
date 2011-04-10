#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Points.h"

struct Points* PointsAlloc(size_t npts) {
    struct Points* p = malloc(sizeof(struct Points));
    assert(p != NULL);

    p->npts = npts;
    p->size = npts*NDIM;
    p->data = malloc(p->size*sizeof(double));
    assert(p->data != NULL);

    return p;
}

void PointsFree(struct Points* p) {
    if (p != NULL) {
        if (p->data != NULL) {
            free(p->data);
        }
        free(p);
    }
}



