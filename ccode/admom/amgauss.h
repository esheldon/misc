#ifndef _ADMOM_GAUSS_HEADER_GUARD
#define _ADMOM_GAUSS_HEADER_GUARD


struct amgauss {
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
};
// use this to keep the structure internally consistent
int amgauss_set(struct amgauss *self,
                double row,
                double col,
                double irr,
                double irc,
                double icc);


#endif
