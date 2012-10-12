#ifndef _ADMOM_GAUSS_HEADER_GUARD
#define _ADMOM_GAUSS_HEADER_GUARD

#define M_TWO_PI   6.28318530717958647693

struct amgauss {
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

    double norm;
};

// use this to keep the structure internally consistent
int amgauss_set(struct amgauss *self,
                double p,
                double row,
                double col,
                double irr,
                double irc,
                double icc);


#endif
