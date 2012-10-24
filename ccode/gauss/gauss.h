#ifndef _GAUSS_HEADER_GUARD
#define _GAUSS_HEADER_GUARD

#include <math.h>

#ifndef M_TWO_PI
#define M_TWO_PI   6.28318530717958647693
#endif

struct gauss {
    double p;
    double row;
    double col;
    double irr;
    double irc;
    double icc;

    // derived quantities
    double det;

    double e1;
    double e2;

    double drr; // irr/det
    double drc;
    double dcc;

    double norm; // 1/( 2*pi*sqrt(det) )
};

#define GAUSS_EVAL(gauss, row, col) ({                         \
    double _u = (row)-(gauss)->row;                            \
    double _v = (col)-(gauss)->col;                            \
                                                               \
    double _chi2 =                                             \
        (gauss)->dcc*_u*_u                                     \
        + (gauss)->drr*_v*_v                                   \
        - 2.0*(gauss)->drc*_u*_v;                              \
                                                               \
    double _val = (gauss)->norm*(gauss)->p*exp( -0.5*_chi2 );  \
                                                               \
    _val;                                                      \
})

// use this to keep the structure internally consistent
int gauss_set(struct gauss *self,
              double p,
              double row,
              double col,
              double irr,
              double irc,
              double icc);


void gauss_print(const struct gauss *self, FILE *stream);

// 0 means without normalization, so the peak is 1
double gauss_lnprob0(const struct gauss *self, double row, double col);

#endif
