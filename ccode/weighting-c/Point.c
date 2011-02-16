#include <stdlib.h>
#include <assert.h>
#include "Point.h"

struct Point* PointAlloc(int ndim) {
    struct Point* p;
    p = malloc(sizeof(struct Point));

    assert(p != NULL);

    p->x = gsl_vector_alloc(ndim);
    p->z = 0;
    p->ndim = ndim;
    return p;
}

void PointFree(struct Point* p) {
    if (p) {
        if (p->x) {
            gsl_vector_free(p->x);
        }
        free(p);
    }
}


int PointValid(struct Point* p) {
    if (!p) {
        return 0;
    }
    if (!p->x) {
        return 0;
    }
    if (!(p->ndim == p->x->size)) {
        return 0;
    }
    return 1;
}

int PointComparable(struct Point* p1, struct Point* p2) {
    if ( !PointValid(p1) || !PointValid(p2) ) {
        fprintf(stderr,"PointComparable: Error: invalid point sent\n");
        return 0;
    }
    if ( p1->ndim != p2->ndim ) {
        fprintf(stderr,"PointComparable: Error: points do not have the same "
                       "dimensions\n");
        return 0;
    }
    return 1;
}

int PointCopy(struct Point* dest, struct Point* src) {

    assert(PointComparable(dest,src));

    dest->z = dest->z;
    int res = gsl_vector_memcpy(dest->x, src->x);
    if (res != 0) {
        fprintf(stderr,"PointCopy: gsl_vector_memcpy failed on Point. Got return "
                       "value: %d\n", res);
        return 0;
    }

    return 1;

}
int PointEqual(struct Point* p1, struct Point* p2) {
    assert(PointComparable(p1,p2));

    for (int i=0; i<p1->ndim; i++) {
        if (p1->x->data[i] != p2->x->data[i]) {
            return 0;
        }
    }

    return 1;
}

double PointDist(struct Point* p1, struct Point* p2) {
    double sum=0;

    assert(PointComparable(p1,p2));
    for (int i=0; i<p1->ndim; i++) {
        double diff = (p1->x->data[i] - p2->x->data[i]);
        sum += diff*diff;
    }
    return sum;
}
