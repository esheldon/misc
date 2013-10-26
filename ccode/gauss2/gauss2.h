#ifndef _GAUSS_HEADER_GUARD
#define _GAUSS_HEADER_GUARD

#include <math.h>

#ifndef M_TWO_PI
#define M_TWO_PI   6.28318530717958647693
#endif

#define GAUSS2_ERROR_NEGATIVE_DET 0x1
#define GAUSS2_ERROR_G_RANGE 0x1

#define GAUSS2_EXP_MAX_CHI2 25

struct gauss2 {
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

    double pnorm; // p*norm
};

// use this to keep the structure internally consistent
void gauss2_set(struct gauss2 *self,
                double p,
                double row,
                double col,
                double irr,
                double irc,
                double icc,
                long *flags);


void gauss2_print(const struct gauss2 *self, FILE *stream);

// 0 means without normalization, so the peak is 1
double gauss2_lnprob0(const struct gauss2 *self, double row, double col);

#define GAUSS2_EVAL(gauss, rowval, colval) ({                   \
    double _u = (rowval)-(gauss)->row;                         \
    double _v = (colval)-(gauss)->col;                         \
                                                               \
    double _chi2 =                                             \
        (gauss)->dcc*_u*_u                                     \
        + (gauss)->drr*_v*_v                                   \
        - 2.0*(gauss)->drc*_u*_v;                              \
                                                               \
    double _val=0.0;                                           \
    if (_chi2 < GAUSS2_EXP_MAX_CHI2) {                         \
        _val = (gauss)->norm*(gauss)->p*expd( -0.5*_chi2 );    \
    }                                                          \
                                                               \
    _val;                                                      \
})

#define GAUSS2_EVAL_SLOW(gauss, rowval, colval) ({             \
    double _u = (rowval)-(gauss)->row;                         \
    double _v = (colval)-(gauss)->col;                         \
                                                               \
    double _chi2 =                                             \
        (gauss)->dcc*_u*_u                                     \
        + (gauss)->drr*_v*_v                                   \
        - 2.0*(gauss)->drc*_u*_v;                              \
                                                               \
    double _val=0.0;                                           \
    if (_chi2 < GAUSS2_EXP_MAX_CHI2) {                         \
        _val = (gauss)->norm*(gauss)->p*exp( -0.5*_chi2 );     \
    }                                                          \
                                                               \
    _val;                                                      \
})


#endif
