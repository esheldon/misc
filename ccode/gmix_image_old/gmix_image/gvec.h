#ifndef _GVEC_HEADER_GUARD
#define _GVEC_HEADER_GUARD

#include "matrix.h"

struct gauss {
    double p;
    double row;
    double col;
    double irr;
    double irc;
    double icc;
    double det;
};

/*
#include "matrix.h"
struct gauss2 {
    double p;
    struct vec2 cen;
    struct mtx2 cov;
};
*/

struct gvec {
    size_t size;
    struct gauss* data;
};

struct gvec *gvec_new(size_t n);
struct gvec *gvec_free(struct gvec *self);
void gvec_set_dets(struct gvec *self);

// this is actually kind of unclear to use in practice since it is easy to
// screw up which parameters go where
void gauss_set(struct gauss* self, 
               double p, 
               double row, 
               double col,
               double irr,
               double irc,
               double icc);

int gvec_copy(struct gvec *self, struct gvec* dest);
void gvec_print(struct gvec *self, FILE* fptr);

// calculate the weighted sum of the moments
//  sum_gi( p*(irr + icc )
double gvec_wmomsum(struct gvec* gvec);

// 0 returned if a zero determinant is found somewhere, else 1
int gvec_wmean_center(const struct gvec* gvec, struct vec2* mu_new);
#endif
